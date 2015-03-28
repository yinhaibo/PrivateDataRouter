//---------------------------------------------------------------------------


#pragma hdrstop

#include "UMasterMessageHandler.h"
#include "LogFileEx.h"
#include "UMasterWorkThread.h"
#include "Tools.h"
#include "UAlgorithm.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

extern LogFileEx logger;


MasterMessageHandler::MasterMessageHandler(
        MasterWorkThread* master,
        ServerClientWorkThread* clientthread,
        Controller* controller)
{
    FQueue = new MsgQueue();
    FMaster = master;
    
    FClientThread = clientthread;
    FController = controller;
    FChannel = FController->registerChannel("", this, 0);
    FChannel->Open();
    FChannel->setPriority(FMaster->Priority);

    receivePos = 0;
    messageLen = 0;
    sendPos = 0;
    sendMessageLen = 0;

    txMsgCnt = 0;
    rxMsgCnt = 0;
    FOffset = 0;
    lastReportTick = 0;

    recvStatus = NET_MASTER_RECV_HEAD;

    FConfig.errorModeIdx = ERROR_MODE_NOERROR_IDX;
    FSeed = 100;
    FMsgCnt = 0;
}
MasterMessageHandler::~MasterMessageHandler()
{
    FController->unregisterChannel(FChannel);
    delete FQueue;
}
//---------------------------------------------------------------------------
void __fastcall MasterMessageHandler::onSendMessage(TCustomWinSocket* client,
        TWinSocketStream *stream)
{
    if (client != NULL ){
        if (sendMessageLen == 0 && !FQueue->Empty()){
            //logger.Log("Pop a Message");//, Queue Count is " + IntToStr(queue->Count()));
            Msg *pmsg = FQueue->Pop();
            //logger.Log("Get a message.");
            // build message
            sendMessageLen = BuildNetMessage(pmsg, sendBuf, NETMESSAGE_MAX_LEN);
            delete pmsg;

            //Generate error data
            ApplyErrorModeOnData(sendBuf, sendMessageLen);
        }
        try{
            if (!client->Connected || (sendMessageLen - sendPos) == 0) return;
            int writeLen = stream->Write(sendBuf + sendPos,
                    sendMessageLen - sendPos);
            if(writeLen < 0){
                logger.Log("Write data error!");
            }
            sendPos += writeLen;
            txMsgCnt += writeLen;
            if (sendPos >= sendMessageLen){
                // a message has sent
                sendMessageLen = 0;
                sendPos = 0;
            }
        }catch(Exception& e){
            logger.Log("Wait for data to write failure." + e.Message);
            FChannel->Close();
            client->Close();
        }
    }
}

int MasterMessageHandler::getDataFromSock(
        TCustomWinSocket* client, TWinSocketStream *stream,
        unsigned char* buffer, int len, int& offset)
{
    if(client != NULL && stream != NULL){
        try{
            if (stream->WaitForData(1)){
                if(!client->Connected)  return CODE_CONN_CLOSE;
                //if(FMaster->isTerminated()) return CODE_THREAD_TERMINATE;

                int rvLen = stream->Read(buffer + offset, len);
                if (rvLen <= 0){
                    LogMsg("Peer close stream.");
                    FChannel->Close();
                    client->Close();
                    return CODE_CONN_CLOSE;
                }
                offset += rvLen;
                rxMsgCnt += rvLen;
                if (offset == len){
                    return len;
                }else{
                    return CODE_NEED_CALL_NEXT;
                }
            }else{
                return 0;
            }
        }catch(Exception& e){
            logger.Log("Master read error." + e.Message);
            FChannel->Close();
            client->Close();
            return NULL;
        }
    }else{
        return -1;
    }
}
        
Msg * __fastcall MasterMessageHandler::onReceiveMessage(TCustomWinSocket* client,
        TWinSocketStream *stream)
{
    int rv;
    Msg *pmsg = NULL;
    switch(recvStatus){
    case NET_MASTER_RECV_HEAD:
        rv = getDataFromSock(client, stream,
                receiveBuf, NET_MESSAGE_HEAD_LEN, FOffset);
        if (rv != NET_MESSAGE_HEAD_LEN){
            break;
        }
        FOffset = 0;
        
        if (!IS_NET_MESSAGE_HEAD(receiveBuf)){
            LogMsg("Invalid messgae head.");
            FChannel->decPriority();
            //Continue to receive message head
            break;
        }
        receivePos = NET_MESSAGE_HEAD_LEN;
        recvStatus = NET_MASTER_RECV_CMD;
        //pass through
    case NET_MASTER_RECV_CMD:
        rv = getDataFromSock(client, stream,
                receiveBuf + receivePos, NET_MESSAGE_CMD_LEN, FOffset);
        if (rv != NET_MESSAGE_CMD_LEN){
            break;
        }
        FOffset = 0;

        if (!IS_NET_MESSAGE_CMD(receiveBuf, receivePos)){
            LogMsg("Invalid messgae command.");
            FChannel->decPriority();
            //Continue to receive message head
            recvStatus = NET_MASTER_RECV_HEAD;
            break;
        }

        receivePos += NET_MESSAGE_CMD_LEN;
        recvStatus = NET_MASTER_RECV_LEN;
        //pass through
    case NET_MASTER_RECV_LEN:
        rv = getDataFromSock(client, stream,
                receiveBuf + receivePos, NETMESSAGE_LEN_LEN, FOffset);
        if (rv != NETMESSAGE_LEN_LEN){
            break;
        }
        FOffset = 0;
        
        messageLen = NETMESSAGE_LEN(receiveBuf, receivePos);
        if (messageLen < NETMESSAGE_MIN_LEN
            || messageLen > NETMESSAGE_MAX_LEN){
            FChannel->decPriority();
            LogMsg("Invalid message len.[" + IntToStr(NETMESSAGE_MIN_LEN)
                + "," + IntToStr(NETMESSAGE_MAX_LEN));
            recvStatus = NET_MASTER_RECV_HEAD;
            break;
        }
        receivePos += NETMESSAGE_LEN_LEN;
        recvStatus = NET_MASTER_RECV_DATA;
        //pass through
    case NET_MASTER_RECV_DATA:
        rv = getDataFromSock(client, stream,
                receiveBuf + receivePos, messageLen
                - NET_MESSAGE_HEAD_LEN - NETMESSAGE_LEN_LEN - NET_MESSAGE_CMD_LEN, FOffset);
        if (rv != messageLen
                - NET_MESSAGE_HEAD_LEN - NETMESSAGE_LEN_LEN - NET_MESSAGE_CMD_LEN){
            break;
        }

        // get a message, neet to post to queue
        pmsg = ResloveNetMessage(receiveBuf, messageLen);
        if (pmsg->validedOK){
            FChannel->incPriority();
        }else{
            FChannel->decPriority();
        }
        // logger
        //FMaster->LogRxMsg(pmsg);

        FOffset = 0;
        recvStatus = NET_MASTER_RECV_HEAD;
        receivePos = 0;
        messageLen = 0;
        return pmsg;
    default:
        LogMsg("Invalid Recv Status.");
    }
            
    if(rv == CODE_NEED_CALL_NEXT){
    }
    return pmsg;
}

void __fastcall MasterMessageHandler::ProcessMessage(TCustomWinSocket* client,
        TWinSocketStream *stream)
{
    try{
        try{
            client->Lock();
            Msg* pmsg = onReceiveMessage(client, stream);
            if (pmsg != NULL){
                if (pmsg->msgtype == MSGTYPE_TAGLIST){
                    FChannel->setAlias((const char**)pmsg->taglist);
                    delete pmsg;
                }else{
                    // logger
                    //FMaster->LogRxMsg(pmsg);
                    // dispathch to work thread
                    FController->dispatchMsg(FChannel, pmsg);
                }
            }
            // write message to peer
            if (client != NULL && client->Connected){
                onSendMessage(client, stream);//
            }
        }__finally{
            client->Unlock();
        }

        // Every second report receive and send message count
        if (::GetTickCount() - lastReportTick >= 100){
            if (FMaster->OnTxMsg != NULL && txMsgCnt > 0){
                FMaster->OnTxMsg(FMaster->Channel, txMsgCnt);
                txMsgCnt = 0;
            }
            if (FMaster->OnRxMsg != NULL && rxMsgCnt > 0){
                FMaster->OnRxMsg(FMaster->Channel, rxMsgCnt);
                rxMsgCnt = 0;
            }
            lastReportTick = ::GetTickCount();
        }

    }catch(...){
        logger.Log("master exception, set null.");
        // write error will disconnect socket
        //mClientPeer = NULL;
    }
}
void MasterMessageHandler::LogMsg(AnsiString msg)
{
    logger.Log("Master Handler[" + IntToStr(GetCurrentThreadId())
        + "]\t" + msg);
}

void MasterMessageHandler::Push(Msg* pmsg)
{
    FQueue->Push(pmsg);
}

Channel* MasterMessageHandler::getChannel()
{
    return FChannel;
}

void MasterMessageHandler::UdateChannelErrorMode(const master_config_t* config)
{
    if (config != NULL){
        if (FConfig.errorModeIdx != config->errorModeIdx){
            FMsgCnt = 0; // Clear count
            FErrorHitIdx = getRandRange(1, FConfig.uniformErrorVal);
        }
        FConfig = *config;
    }
}

void MasterMessageHandler::ApplyErrorModeOnData(unsigned char* buffer, int len)
{
    switch(FConfig.errorModeIdx){
    case ERROR_MODE_NOERROR_IDX:
        break;
    case ERROR_MODE_UNIFORM_IDX:
        FMsgCnt++;
        if (FMsgCnt == FErrorHitIdx){
            buffer[FErrorHitIdx % len] = 0; // Set error data
            LogMsg("Generate a error in message position " + IntToStr(FErrorHitIdx % len));
        }else if(FMsgCnt >= FConfig.uniformErrorVal){
            FMsgCnt = 0;
            // Gernerate a new rand(Uniform distribution) index using
            // current error val, as say 1000
            // The error will be insert, for say in 456 messages
            FErrorHitIdx = getRandRange(1, FConfig.uniformErrorVal);
            LogMsg("Create a error mode in " + IntToStr(FErrorHitIdx));
        }
        break;
    case ERROR_MODE_POSSION_IDX:
        FErrorHitIdx = getRandRange(1, 100);
        // In possion mode, the error distribution will generate every sending
        // the rand range will generate a data between 1 - 100,
        // and as possion distribution
        if (FErrorHitIdx > FConfig.possionErrorVal){
            buffer[FErrorHitIdx % len] = 0; // Set error data
        }
        break;
    default:
        break;
    }
}

int MasterMessageHandler::getRandRange(int from , int to)
{
    if (FConfig.errorModeIdx == ERROR_MODE_UNIFORM_IDX){
        return from + 0.5 + ran1(&FSeed) * (to - from);
    }else if(FConfig.errorModeIdx == ERROR_MODE_POSSION_IDX){
        return from + 0.5 + poidev((to - from)/2, &FSeed);
    }else{
        return from;
    }
}
