//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <Math.hpp>
#include <stdio.h>
#include "UWorkThread.h"
#pragma package(smart_init)


#include "LogFileEx.h"
#include "Tools.h"

//Logger
extern LogFileEx logger;

#include "UAlgorithm.h"

//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall WorkThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall WorkThread::WorkThread(const device_config_t* pDevCfg)
    : TThread(true), mpDevCfg(pDevCfg)
{
    errorMsgPerMsg = -1;
    respMsgCnt = 0;
    reconnectTick = 0;
    FPeerReady = false;
    sendSeq = 1; //reset send sequence number
    mRecvLen = 0;

    memset((void*)&mMessage, 0, sizeof(mMessage));
    memset((void*)&mEOFMessage, 0, sizeof(mEOFMessage));
    CREATE_MESSAGE(mMessage, pDevCfg->head, pDevCfg->tag,
        1, pDevCfg->message);
    CREATE_MESSAGE(mEOFMessage, pDevCfg->head, pDevCfg->tag,
        1, pDevCfg->eofMessage); 
    //memcpy(&mRespMsg, prespMsg, sizeof(message_t));
    //memcpy(&mRespErrMsg, &mRespMsg, sizeof(message_t));
    //mRespErrMsg.len = 0;
    //mRespErrMsg.content[0] = '\0';
    memset(&mReceiveMsgBuf, 0, sizeof(mReceiveMsgBuf));
    memset(&mSendMsgBuf, 0, sizeof(mSendMsgBuf));
    
    mMsgStatus = RECV_MSG_STATUS_HEAD;
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::Start()
{
    mIsRunning = true;
    try{
        onStart();
        this->Resume();
        if (FOnOpenChannel != NULL){
            FOnOpenChannel(this, true);
        }
    }catch(...){
        if (FOnOpenChannel != NULL){
            FOnOpenChannel(this, false);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::Stop()
{
    mIsRunning = false;
    try{
        onStop();
        if (FOnCloseChannel != NULL){
            FOnCloseChannel(this, true);
        }
    }catch(...){
        if (FOnCloseChannel != NULL){
            FOnCloseChannel(this, false);
        }
    }
}
//---------------------------------------------------------------------------
int __fastcall WorkThread::getRandRange(int from , int to)
{
    return from + 0.5 + ran1(&FSeed) * (to - from);
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::Execute()
{
    unsigned int lastReportTick = 0;
    unsigned int txMsgCnt = 0;
    unsigned int rxMsgCnt = 0;
    unsigned int errMsgCnt = 0;
    unsigned int requestTick = 0;
    unsigned int lastRequestTick = 0;
    message_t receiveMsg;

    msgStatus = MESSAGE_SEND_MESSAGE; // At first, Send a message with inforation
    //---- Place thread code here ----
    while(!this->Terminated){
        while (!this->Terminated && mIsRunning){
            if (reconnectTick > 0){
                if ((GetTickCount() - reconnectTick) > 3000){
                    onReStart();
                }
            }
            if (FActiveMode){
            ////////////////////////////////////////////////
            //Active
            ////////////////////////////////////////////////
                // In active mode, the thread will send request the message
                // instead of receive and response the message
                if (::GetTickCount() - lastRequestTick >= requestTick && FPeerReady){
                    // save current tick
                    lastRequestTick = ::GetTickCount();
                    // generate next step's delay tick
                    requestTick = getRandRange(mpDevCfg->delayFrom, mpDevCfg->delayTo);
                    // Send message

                    if (msgStatus == MESSAGE_SEND_MESSAGE)
                    {
                        LogMsg("Tx, Seq:" + IntToStr(sendSeq) + " Message");
                        if (mMessage.seq != sendSeq){
                            // Only renew the message while send sequence has changed.
                            fillMessageStruct(mMessage);
                        }
                        if (onSendMessage(mMessage, 0)){
                            txMsgCnt++;
                        }
                    }else{
                        LogMsg("Tx, Seq:" + IntToStr(sendSeq) + " EOF Message");
                        if (mEOFMessage.seq != sendSeq){
                            // Only renew the message while send sequence has changed.
                            fillMessageStruct(mEOFMessage);
                        }
                        if (onSendMessage(mEOFMessage, 0)){
                            txMsgCnt++;
                        }
                    }
                }
                // receive message
                message_t* pmsg = onReceiveMessage();
                if (pmsg != NULL){
                    memcpy(&receiveMsg, pmsg, sizeof(message_t));
                    if (bMessageOK){
                        // There is a response message while their has a same tag field.
                        if (pmsg->tag == mpDevCfg->tag){
                            if (msgStatus == MESSAGE_SEND_MESSAGE)
                            {
                                if (receiveMsg.seq == mMessage.seq &&
                                    receiveMsg.clen == mMessage.clen &&
                                    receiveMsg.crc16 == mMessage.crc16)
                                {
                                    msgStatus = MESSAGE_SEND_EOFMESSAGE;
                                    fillMessageStruct(mEOFMessage);
                                    //Send correct message immediately
                                    onSendMessage(mEOFMessage, 0);
                                }else{
                                    // Error seq or len or content, resend
                                    onSendMessage(mMessage, 0);
                                    errMsgCnt++;
                                }
                            }else{
                                if (receiveMsg.seq == mEOFMessage.seq &&
                                    receiveMsg.clen == mEOFMessage.clen &&
                                    receiveMsg.crc16 == mEOFMessage.crc16)
                                {
                                    rxMsgCnt++;
                                    //Send success
                                    LogMsg("Rx, Seq:" + IntToStr(sendSeq));
                                    sendSeq++; //Success
                                    msgStatus = MESSAGE_SEND_MESSAGE;
                                    // Wait a time space to send
                                }else{
                                    // Error seq or len or content, resend
                                    onSendMessage(mEOFMessage, 0);
                                    errMsgCnt++;
                                }
                            }
                        }else{
                            // Resonse message
                            // Just copy that and back it.
                            if (FPeerReady){
                                respMsgCnt++;
                                LogMsg("Response message");
                                onSendMessage(receiveMsg, 0);
                            }
                            txMsgCnt++;
                        }
                    }else{
                        LogMsg("Rx Error, Seq:" + IntToStr(sendSeq) + "->" + StreamToText(receiveMsg.content, sizeof(receiveMsg.len)));
                        errMsgCnt++;
                    }
                }
            }else{
            ////////////////////////////////////////////////
            //Passive
            ////////////////////////////////////////////////

                // In passive mode, the thread receive message from peer
                // and check and response message.
                // In passive mode, these is no error message.
                message_t* pmsg = onReceiveMessage();
                if (pmsg != NULL){
                    rxMsgCnt++;
                    memcpy(&receiveMsg, pmsg, sizeof(message_t));
                    if (bMessageOK){
                        if (FPeerReady){
                            LogMsg("Response message");
                            respMsgCnt++;
                            onSendMessage(receiveMsg, 0);
                        }
                        txMsgCnt++;
                    }else{
                        LogMsg("!!!IMPOSSIBLE!!!");
                        errMsgCnt++;
                    }
                }
            }

            // Every second report receive and send message count
            if (::GetTickCount() - lastReportTick >= 100){
                if (OnTxMsg != NULL){
                    OnTxMsg(this, txMsgCnt);
                    txMsgCnt = 0;
                }
                if (OnRxMsg != NULL){
                    OnRxMsg(this, rxMsgCnt);
                    rxMsgCnt = 0;
                }
                if (OnErrMsg != NULL){
                    OnErrMsg(this, errMsgCnt);
                    errMsgCnt = 0;
                }
                lastReportTick = ::GetTickCount();
            }
            Sleep(10);
        }

        // Every second report receive and send message count
        if (::GetTickCount() - lastReportTick >= 100){
            if (OnTxMsg != NULL){
                OnTxMsg(this, txMsgCnt);
                txMsgCnt = 0;
            }
            if (OnRxMsg != NULL){
                OnRxMsg(this, rxMsgCnt);
                rxMsgCnt = 0;
            }
            if (OnErrMsg != NULL){
                OnErrMsg(this, errMsgCnt);
                errMsgCnt = 0;
            }
            lastReportTick = ::GetTickCount();
        }
        if (!mIsRunning){
            Sleep(10);
        }            
    }
}
//---------------------------------------------------------------------------
bool __fastcall WorkThread::onSendMessage(message_t& msg, int error)
{
    int wpos = 0;
    int headpartlen = offsetof(message_t, content);
    memcpy(sendRawBuff + wpos, &msg.head, headpartlen);
    wpos += headpartlen;
    memcpy(sendRawBuff + wpos, msg.content, msg.clen);
    wpos += msg.clen;
    //CRC16
    msg.crc16 = crc16_cal(sendRawBuff, msg.len - MESSAGE_CRC_LEN);
    memcpy(sendRawBuff + wpos, &msg.crc16, sizeof(msg.crc16));
    wpos += sizeof(msg.crc16);
    
    sendPos = 0;
    if (error > 0){
        // Write a error message
        sendRawBuff[error % wpos] = ~sendRawBuff[error % wpos];
    }
    int sendLen = sendData(sendRawBuff, wpos);
    return (sendLen == wpos);
}
//---------------------------------------------------------------------------
message_t* __fastcall WorkThread::onReceiveMessage()
{
    int rvRecv;
    unsigned short rvCRC16;
    unsigned short head;
    if(!(hasDataRead)){
        return NULL;
    }
    switch(mMsgStatus){
    case RECV_MSG_STATUS_HEAD:
        bMessageOK = false; // Message has no OK
        // Receive head data, including seq, timestampt and len data.
        rvRecv = receiveData((unsigned char*)&recvRawBuff, MESSAGE_HEAD_LEN);
        if (rvRecv != MESSAGE_HEAD_LEN){
            return NULL;
        }
        if (*(unsigned short*)recvRawBuff == mMessage.head){
            //Success receive head information and skip to next step
            receivePos = 0;
            mMsgStatus = RECV_MSG_STATUS_LEN;
        }else{
            return NULL; // Need to read a vaild message head
        }
        //pass through
    case RECV_MSG_STATUS_LEN:
        // Receive message len
        rvRecv = receiveData((unsigned char*)&recvRawBuff + MESSAGE_HEAD_LEN, MESSAGE_LEN_LEN);
        if (rvRecv == -1){
            mMsgStatus = RECV_MSG_STATUS_HEAD;
        }
        if (rvRecv != MESSAGE_LEN_LEN){
            return NULL;
        }else{
            receivePos = 0;
            mMsgStatus = RECV_MSG_STATUS_DATA;
            mRecvLen = *(unsigned short*)(recvRawBuff + MESSAGE_HEAD_LEN);
        }
        //pass through
    case RECV_MSG_STATUS_DATA:
        rvRecv = receiveData((unsigned char*)&recvRawBuff + MESSAGE_HEAD_LEN + MESSAGE_LEN_LEN,
            mRecvLen - MESSAGE_HEAD_LEN - MESSAGE_LEN_LEN);
        if (rvRecv == -1){
            mMsgStatus = RECV_MSG_STATUS_HEAD;
        }
        if (rvRecv != mRecvLen - MESSAGE_HEAD_LEN - MESSAGE_LEN_LEN){
            return NULL;
        }
        //and skip to next step
        receivePos = 0;
        mMsgStatus = RECV_MSG_STATUS_HEAD;

        // resolve message to struct messgae
        memcpy(&mReceiveMsgBuf, recvRawBuff, offsetof(message_t, content));
        memset(mReceiveMsgBuf.content, 0, sizeof(mReceiveMsgBuf.content));
        memcpy(mReceiveMsgBuf.content, recvRawBuff + offsetof(message_t, content),
            mReceiveMsgBuf.clen);
        memcpy(&mReceiveMsgBuf.crc16, recvRawBuff +
            mReceiveMsgBuf.len - MESSAGE_CRC_LEN,
            MESSAGE_CRC_LEN);

        //Success receive data information and check the message
        rvCRC16 = crc16_cal(recvRawBuff, mReceiveMsgBuf.len - MESSAGE_CRC_LEN);
        if (rvCRC16 == mReceiveMsgBuf.crc16){
            // Verified success
            bMessageOK = true; // Received a correct message.
        }
        return &mReceiveMsgBuf;

    }
    return NULL;
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::fillMessageStruct(message_t& msg)
{
    if (msgStatus == MESSAGE_SEND_MESSAGE)
    {
        CREATE_MESSAGE(msg, mpDevCfg->head, mpDevCfg->tag,
            sendSeq, mpDevCfg->message);
    }else{
        CREATE_MESSAGE(msg, mpDevCfg->head, mpDevCfg->tag,
            sendSeq, mpDevCfg->eofMessage);
    }
}

bool __fastcall WorkThread::getReconnect()
{
    return autoReconnect;
}

void __fastcall WorkThread::setReconnect(bool value)
{
    autoReconnect = value;
}

void WorkThread::LogMsg(AnsiString msg)
{
    static char messagebuf[512];
    snprintf(messagebuf, 512, "%s[%6d]\t%s", FName, this->ThreadID, msg);
    logger.Log(messagebuf);
    //logger.Log(FName + "[" + IntToStr(this->ThreadID) + "]\t" + msg);
}
//---------------------------------------------------------------------------




void __fastcall WorkThreadMessage::SetErrorMsg(AnsiString value)
{
    if(FErrorMsg != value) {
        FErrorMsg = value;
    }
}
AnsiString __fastcall WorkThreadMessage::GetErrorMsg()
{
    return FErrorMsg;
}

void __fastcall WorkThreadMessage::SetRxMsgCnt(int value)
{
    if(FRxMsgCnt != value) {
        FRxMsgCnt = value;
    }
}
int __fastcall WorkThreadMessage::GetRxMsgCnt()
{
    return FRxMsgCnt;
}

void __fastcall WorkThreadMessage::SetTxMsgCnt(int value)
{
    if(FTxMsgCnt != value) {
        FTxMsgCnt = value;
    }
}
int __fastcall WorkThreadMessage::GetTxMsgCnt()
{
    return FTxMsgCnt;
}

void __fastcall WorkThreadMessage::SetErrMsgCnt(int value)
{
    if(FErrMsgCnt != value) {
        FErrMsgCnt = value;
    }
}
int __fastcall WorkThreadMessage::GetErrMsgCnt()
{
    return FErrMsgCnt;
}


