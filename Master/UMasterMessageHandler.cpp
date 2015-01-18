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
    recvLen = 0;
    lastReportTick = 0;
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
            sendMessageLen = BuildNetMessage(pmsg, sendBuf, MESSAGE_BUF_LEN);
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
Msg * __fastcall MasterMessageHandler::onReceiveMessage(TCustomWinSocket* client,
        TWinSocketStream *stream)
{
    if(client != NULL && stream != NULL){
        if (stream->WaitForData(1)){
            if(!client->Connected)  return NULL;
            try{
                if (messageLen == 0){
                    recvLen = stream->Read(receiveBuf + receivePos,
                        NETMESSAGE_HEADLEN - receivePos);
                }else{
                    recvLen = stream->Read(receiveBuf + receivePos,
                                    messageLen - receivePos);
                }
            }catch(...){
                logger.Log("Master read error.");
                client->Close();
                return NULL;
            }
            if (recvLen > 0){

                receivePos += recvLen;
                rxMsgCnt += recvLen;
                if (receivePos == NETMESSAGE_HEADLEN){
                    messageLen = NETMESSAGE_HEAD(receiveBuf, 0);
                }else if (receivePos == messageLen){
                    // get a message, neet to post to queue
                    Msg *pmsg = ResloveNetMessage(receiveBuf, messageLen);

                    messageLen = 0;
                    receivePos = 0;

                    return pmsg;
                }
            }else{
                client->Close();
            }
        }
    }
    return NULL;
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