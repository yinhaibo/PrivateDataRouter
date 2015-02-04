//---------------------------------------------------------------------------


#pragma hdrstop

#include "UMasterMessageHandler.h"
#include "LogFileEx.h"
#include "UMasterWorkThread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

extern LogFileEx logger;


MasterMessageHandler::MasterMessageHandler(
        MasterWorkThread* master)
{
    this->FMaster = master;
    queue = FMaster->GetQueue();

    receivePos = 0;
    messageLen = 0;
    sendPos = 0;
    sendMessageLen = 0;

    txMsgCnt = 0;
    rxMsgCnt = 0;
    FOffset = 0;
    lastReportTick = 0;

    recvStatus = NET_MASTER_RECV_HEAD;
}

//---------------------------------------------------------------------------
void __fastcall MasterMessageHandler::onSendMessage(TCustomWinSocket* client,
        TWinSocketStream *stream)
{
    if (client != NULL ){
        if (sendMessageLen == 0 && !queue->Empty()){
            //logger.Log("Pop a Message");//, Queue Count is " + IntToStr(queue->Count()));
            Msg *pmsg = queue->Pop();
            //logger.Log("Get a message.");
            // build message
            sendMessageLen = BuildNetMessage(pmsg, sendBuf, NETMESSAGE_MAX_LEN);
            // logger
            FMaster->LogTxMsg(pmsg);
            delete pmsg;
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
        }catch(...){
            logger.Log("Wait for data to write failure.");
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
                if(FMaster->isTerminated()) return CODE_THREAD_TERMINATE;

                int rvLen = stream->Read(buffer + offset, len);
                if (rvLen < 0){
                    LogMsg("Peer close stream.");
                    client->Close();
                    return CODE_CONN_CLOSE;
                }
                offset += rvLen;
                if (offset == len){
                    return len;
                }else{
                    return CODE_NEED_CALL_NEXT;
                }
            }else{
                return 0;
            }
        }catch(...){
            logger.Log("Master read error.");
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
            //Continue to receive message head
            break;
        }
        receivePos = NET_MESSAGE_HEAD_LEN;
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
            LogMsg("Invalid message len.[" + IntToStr(NETMESSAGE_MIN_LEN)
                + "," + IntToStr(NETMESSAGE_MAX_LEN));
            break;
        }
        receivePos += NETMESSAGE_LEN_LEN;
        recvStatus = NET_MASTER_RECV_DATA;
        //pass through
    case NET_MASTER_RECV_DATA:
        rv = getDataFromSock(client, stream,
                receiveBuf + receivePos, messageLen
                - NET_MESSAGE_HEAD_LEN - NETMESSAGE_LEN_LEN, FOffset);
        if (rv != messageLen
                - NET_MESSAGE_HEAD_LEN - NETMESSAGE_LEN_LEN){
            break;
        }

        // get a message, neet to post to queue
        pmsg = ResloveNetMessage(receiveBuf, messageLen);
        // logger
        FMaster->LogRxMsg(pmsg);

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
                // logger
                FMaster->LogRxMsg(pmsg);
                // dispathch to work thread
                FMaster->dispatchMsg(pmsg);
                delete pmsg;
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
                FMaster->OnTxMsg(FMaster, txMsgCnt);
                txMsgCnt = 0;
            }
            if (FMaster->OnRxMsg != NULL && rxMsgCnt > 0){
                FMaster->OnRxMsg(FMaster, rxMsgCnt);
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