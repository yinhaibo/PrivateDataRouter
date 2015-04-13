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

__fastcall WorkThread::WorkThread(device_config_t* pDevCfg)
    : TThread(true), mpDevCfg(pDevCfg)
{
    errorMsgPerMsg = -1;
    respMsgCnt = 0;
    reconnectTick = 0;
    FPeerReady = false;
    isEnableWrite = false;
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
    sendSeq = 0;//Reset the sequence of transmission
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

    unsigned int txMsgCnt = 0;
    unsigned int rxMsgCnt = 0;
    unsigned int errMsgCnt = 0;
    DWORD lastReportTick = 0;
    DWORD requestTick = 0;
    DWORD lastRequestTick = 0;
    message_t receiveMsg;

    sendSeq = 1;
    mpDevCfg->sendSeq = 0; // Set a invalid sequence

    msgStatus = MESSAGE_SEND_MESSAGE; // At first, Send a message with inforation
    //---- Place thread code here ----
    while(!this->Terminated){
        while (!this->Terminated && mIsRunning){
            if (reconnectTick > 0){
                if ((GetTickCount() - reconnectTick) > 3000){
                    onReStart();
                }
            }
            // In active mode, the thread will send request the message
            // actively on random time depends delay parameters
            if (FActiveMode
                && ::GetTickCount() - lastRequestTick >= requestTick
                && FPeerReady && isEnableWrite
                && sendSeq <= FMaxMessageSend){
                // save current tick
                lastRequestTick = ::GetTickCount();
                // generate next step's delay tick
                requestTick = getRandRange(mpDevCfg->delayFrom, mpDevCfg->delayTo);

                // Send message, Message or EOF Message
                if (msgStatus == MESSAGE_SEND_MESSAGE)
                {
                    LogMsg("Tx Message, Seq:" + IntToStr(sendSeq));
                    if (mMessage.seq != sendSeq){
                        // Only renew the message while send sequence has changed.
                        fillMessageStruct(mMessage);
                    }
                    if (onSendMessage(mMessage, 0)){
                        txMsgCnt++;
                    }
                }else{
                    LogMsg("Tx EOF Message, Seq:" + IntToStr(sendSeq));
                    if (mEOFMessage.seq != sendSeq){
                        // Only renew the message while send sequence has changed.
                        fillMessageStruct(mEOFMessage);
                    }
                    if (onSendMessage(mEOFMessage, 0)){
                        mpDevCfg->msgTxCnt++;
                        txMsgCnt++;
                    }
                }
            }
            // receive message
            message_t* pmsg = onReceiveMessage();
            if (pmsg != NULL){
                memcpy(&receiveMsg, pmsg, sizeof(message_t));
                rxMsgCnt++;
                if (bMessageOK){
                    LogMsg(pmsg, "Receive message OK");
                    // There is a response message while their has a same tag field.
                    if (pmsg->tag == mpDevCfg->tag){
                        if (msgStatus == MESSAGE_SEND_MESSAGE)
                        {
                            ///////////MESSAGE////////////
                            if (receiveMsg.seq == mMessage.seq &&
                                receiveMsg.clen == mMessage.clen &&
                                receiveMsg.crc16 == mMessage.crc16)
                            {
                                LogMsg("Rx Message, Seq:" + IntToStr(sendSeq));
                                // Message has sent successfullly
                                // and Send EOF message immediatelly
                                msgStatus = MESSAGE_SEND_EOFMESSAGE;
                                fillMessageStruct(mEOFMessage);
                                if (onSendMessage(mEOFMessage, 0)){
                                    txMsgCnt++;
                                }
                            }else{
                                // Error seq or len or content.
                                // Message will resent after delay works.
                                errMsgCnt++;
                                LogMsg("Rx Error Message, Seq:" + IntToStr(sendSeq));
                            }
                        }else{
                            ///////////EOF MESSAGE////////////
                            if (receiveMsg.seq == mEOFMessage.seq &&
                                receiveMsg.clen == mEOFMessage.clen &&
                                receiveMsg.crc16 == mEOFMessage.crc16)
                            {
                                //Send success
                                LogMsg("Rx EOF Message, Seq:" + IntToStr(sendSeq));
                                sendSeq++; //Increase message sequence
                                if (sendSeq == 0){
                                    sendSeq++;
                                }
                                msgStatus = MESSAGE_SEND_MESSAGE;
                            }else{
                                // Error seq or len or content.
                                // Message will resent after delay works.
                                errMsgCnt++;
                                LogMsg("Rx Error EOF Message, Seq:" + IntToStr(mEOFMessage.seq));
                            }
                        }
                    }else{
                        // Resonse message
                        // Just copy that and back it.
                        if (FPeerReady){
                            LogMsg("Response message, Seq:" + IntToStr(pmsg->seq));
                            if (onSendMessage(receiveMsg, 0)){
                                txMsgCnt++;
                            }
                        }
                    }
                }else{
                    LogMsg("Rx Error, Seq:" + IntToStr(sendSeq) + "->" + StreamToText(receiveMsg.content, sizeof(receiveMsg.len)));
                    errMsgCnt++;
                }
            }

            updateUIEvent(txMsgCnt, rxMsgCnt, errMsgCnt, lastReportTick);
            Sleep(10);
        }

        updateUIEvent(txMsgCnt, rxMsgCnt, errMsgCnt, lastReportTick);
        if (!mIsRunning){
            Sleep(10);
        }            
    }
    LogMsg("Work thread exit...");
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::updateUIEvent(
    unsigned int& txMsgCnt,
    unsigned int& rxMsgCnt,
    unsigned int& errMsgCnt,
    DWORD& lastReportTick)
{
    // Every second report receive and send message count
    if (::GetTickCount() - lastReportTick >= 200){
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
        if (FOnMsgSeqUpdate != NULL){
            OnMsgSeqUpdate(this, sendSeq);
        }
        lastReportTick = ::GetTickCount();
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
    LogMsg(&msg, "Send message");
    int sendLen = sendData(sendRawBuff, wpos);
    bool isSendOK = (sendLen == wpos);
    
    if (isSendOK && mpDevCfg->tag == msg.tag){
        if (mpDevCfg->sendSeq == 0){
            LogMsg("Start to record send message.");
            // First time
            mpDevCfg->msgMsgSent = 1;
            mpDevCfg->sendSeq = msg.seq;
        }else if (mpDevCfg->sendSeq == msg.seq){
            mpDevCfg->dcResendCnt.val++;
            LogMsg("Seq:" + IntToStr(mpDevCfg->sendSeq) + " send " +
                IntToStr(mpDevCfg->dcResendCnt.val) + "Times");
        }else{            
            // Calc min, max and avg
            mpDevCfg->msgMsgSent++;
            mpDevCfg->dcResendCnt.avg =
                ((mpDevCfg->msgMsgSent - 1.0f) * mpDevCfg->dcResendCnt.avg
                    + mpDevCfg->dcResendCnt.val)
                / mpDevCfg->msgMsgSent;
            if (mpDevCfg->dcResendCnt.min > mpDevCfg->dcResendCnt.val){
                mpDevCfg->dcResendCnt.min = mpDevCfg->dcResendCnt.val;
            }
            if (mpDevCfg->dcResendCnt.max < mpDevCfg->dcResendCnt.val){
                mpDevCfg->dcResendCnt.max = mpDevCfg->dcResendCnt.val;
            }
            mpDevCfg->sendSeq = msg.seq;  // Update send sequence
            mpDevCfg->dcResendCnt.val = 0;// Reset resend value
            LogMsg("new Seq:" + IntToStr(mpDevCfg->sendSeq) + " send. Message sent:" + IntToStr(mpDevCfg->msgMsgSent));
        }
        mpDevCfg->sendTick = GetTickCount();
        mpDevCfg->msgTxCnt++;
    }
    return isSendOK;
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
            if (mRecvLen > MAX_RAW_BUFFER_SIZE || mRecvLen > sizeof(message_t)){
                LogMsg("Error message length, re-scan message head.");
                mMsgStatus = RECV_MSG_STATUS_HEAD;
                return NULL;
            }
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
        if (mReceiveMsgBuf.clen > MAX_MESSAGE_LEN){
            LogMsg("Error message content length, re-scan message head.");
            return NULL;
        }
        memset(mReceiveMsgBuf.content, 0, sizeof(mReceiveMsgBuf.content));
        memcpy(mReceiveMsgBuf.content, recvRawBuff + offsetof(message_t, content),
            mReceiveMsgBuf.clen);
        memcpy(&mReceiveMsgBuf.crc16, recvRawBuff +
            mReceiveMsgBuf.len - MESSAGE_CRC_LEN,
            MESSAGE_CRC_LEN);

        //Success receive data information and check the message
        rvCRC16 = crc16_cal(recvRawBuff, mReceiveMsgBuf.len - MESSAGE_CRC_LEN);

        if (mpDevCfg->tag == mReceiveMsgBuf.tag){
            //calc response time
            mpDevCfg->dcRespTime.val = GetTickCount() - mpDevCfg->sendTick; 
            mpDevCfg->dcRespTime.avg =
                ((mpDevCfg->msgTxCnt - 1.0f) * mpDevCfg->dcRespTime.avg
                    + mpDevCfg->dcRespTime.val)
                / mpDevCfg->msgTxCnt;
            if (mpDevCfg->dcRespTime.min > mpDevCfg->dcRespTime.val){
                mpDevCfg->dcRespTime.min = mpDevCfg->dcRespTime.val;
            }
            if (mpDevCfg->dcRespTime.max < mpDevCfg->dcRespTime.val){
                mpDevCfg->dcRespTime.max = mpDevCfg->dcRespTime.val;
            }

            if (rvCRC16 == mReceiveMsgBuf.crc16){
                mpDevCfg->msgRxCnt++;
            }else{
                mpDevCfg->msgErrCnt++;
            }
        }

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
void WorkThread::LogMsg(message_t* pmsg, AnsiString text)
{
    char buffer[200];
    snprintf(buffer, 200, "[%s]Tag:%u, Seq:%u, CRC:%04x -- %s", FName.c_str(),
        pmsg->tag, pmsg->seq, pmsg->crc16,
        text.c_str());
    logger.Log(buffer);
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


