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

AnsiString GetDistributionDesc(distribution_t distri)
{
    switch(distri){
    case UNIFORM_DISTRIBUTION:
        return "Uniform";
    case POISSON_DISTRIBUTION:
        return "Poisson";
    case NO_ERROR_DISTRIBUTION:
    default:
        return "No Error";
    }
}

distribution_t GetDistributionFromDesc(AnsiString desc)
{
    if (desc == "Uniform"){
        return UNIFORM_DISTRIBUTION;
    }else if(desc == "Poisson"){
        return POISSON_DISTRIBUTION;
    }else{
        return NO_ERROR_DISTRIBUTION;
    }
}
//---------------------------------------------------------------------------
WorkParameter::WorkParameter(const WorkParameter& value)
{
    this->Mode = value.Mode;
    this->Configure = value.Configure;
    this->DelayFrom = value.DelayTo;
    this->DelayTo = value.DelayTo;
    this->ErrorFrom = value.ErrorFrom;
    this->ErrorTo = value.ErrorTo;
    this->HeadHex = value.HeadHex;
    this->TailHex = value.TailHex;
}
WorkParameter WorkParameter::operator=(const WorkParameter& value)
{
    this->Mode = value.Mode;
    this->Configure = value.Configure;
    this->DelayFrom = value.DelayTo;
    this->DelayTo = value.DelayTo;
    this->ErrorFrom = value.ErrorFrom;
    this->ErrorTo = value.ErrorTo;
    this->HeadHex = value.HeadHex;
    this->TailHex = value.TailHex;

    return *this;
}
WorkMode __fastcall  WorkParameter::fGetMode(void) const
{
    return mMode;
}

void        __fastcall  WorkParameter::fSetMode(WorkMode val)
{
    if (val != mMode){
        mMode = val;
    }
}

AnsiString  __fastcall  WorkParameter::fGetConfigure(void) const
{
    return mConfigure;
}

void        __fastcall  WorkParameter::fSetConfigure(AnsiString val)
{
    if (val != mConfigure){
        mConfigure = val;
    }
}

int         __fastcall  WorkParameter::fGetDelayFrom(void) const
{
    return delayFrom;
}

void        __fastcall  WorkParameter::fSetDelayFrom(int val)
{
    if (val != delayFrom){
        delayFrom = val;
    }
}

int         __fastcall  WorkParameter::fGetDelayTo(void) const
{
    return delayTo;
}

void        __fastcall  WorkParameter::fSetDelayTo(int val)
{
    if (val != delayTo){
        delayTo = val;
    }
}

int         __fastcall  WorkParameter::fGetErrorFrom(void) const
{
    return errorFrom;
}


void        __fastcall  WorkParameter::fSetErrorFrom(int val)
{
    if (val != errorFrom){
        errorFrom = val;
    }
}

int         __fastcall  WorkParameter::fGetErrorTo(void) const
{
    return errorTo;
}


void        __fastcall  WorkParameter::fSetErrorTo(int val)
{
    if (val != errorTo){
        errorTo = val;
    }
}
void __fastcall WorkParameter::SetRequestMsg(AnsiString value)
{
    if(FRequestMsg != value) {
        FRequestMsg = value;
    }
}
AnsiString __fastcall WorkParameter::GetRequestMsg()
{
    return FRequestMsg;
}

void __fastcall WorkParameter::SetResponseMsg(AnsiString value)
{
    if(FResponseMsg != value) {
        FResponseMsg = value;
    }
}
AnsiString __fastcall WorkParameter::GetResponseMsg()
{
    return FResponseMsg;
}

void __fastcall WorkParameter::SetHeadHex(AnsiString value)
{
    if(FHeadHex != value) {
        FHeadHex = value;
    }
}
AnsiString __fastcall WorkParameter::GetHeadHex() const
{
    return FHeadHex;
}

void __fastcall WorkParameter::SetTailHex(AnsiString value)
{
    if(FTailHex != value) {
        FTailHex = value;
    }
}
AnsiString __fastcall WorkParameter::GetTailHex() const
{
    return FTailHex;
}
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

__fastcall WorkThread::WorkThread(const WorkParameter& param,
    const message_t* preqMsg,
    const message_t* prespMsg)
    : TThread(true), mParam(param)
{
    errorMsgPerMsg = -1;
    respMsgCnt = 0;
    reconnectTick = 0;
    FPeerReady = false;
    sendSeq = 1; //reset send sequence number
    
    memcpy(&mReqMsg, preqMsg, sizeof(message_t));
    memcpy(&mRespMsg, prespMsg, sizeof(message_t));
    memcpy(&mRespErrMsg, &mRespMsg, sizeof(message_t));
    mRespErrMsg.len = 0;
    mRespErrMsg.content[0] = '\0';

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
    if (FErrorDistribution == UNIFORM_DISTRIBUTION){
        return from + 0.5 + ran1(&FSeed) * (to - from);
    }else if(FErrorDistribution == POISSON_DISTRIBUTION){
        return from + 0.5 + poidev((to - from)/2, &FSeed);
    }else{
        return from;
    }
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
    //---- Place thread code here ----
    while(!this->Terminated){

        // reinitiliaze error factor
        if (errorMsgPerMsg == -1){
            // Generate error parameter in random way
            errorMsgPerMsg = getRandRange(mParam.ErrorFrom, mParam.ErrorTo);
        }
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
                    requestTick = getRandRange(mParam.DelayFrom, mParam.DelayTo);
                    // Send message
                    LogMsg("Tx, Seq:" + IntToStr(sendSeq));
                    mReqMsg.seq = sendSeq & 0xFF;
                    fillMessageStruct(mReqMsg);
                    if (onSendMessage(mReqMsg)){
                        txMsgCnt++;
                    }
                }
                // receive message
                message_t* pmsg = onReceiveMessage();
                if (pmsg != NULL){
                    rxMsgCnt++;
                    memcpy(&receiveMsg, pmsg, sizeof(message_t));
                    // when the message is response messsage, increment seqence number of sending
                    // when the message is request message, is a active message, need response it.
                    if (memcmp(&receiveMsg, &mRespMsg, sizeof(message_t)) == 0){
                        //Send success
                        LogMsg("Rx, Seq:" + IntToStr(sendSeq));
                        sendSeq++; //Success
                    }else if(memcmp(&receiveMsg, &mReqMsg, sizeof(message_t)) == 0){
                        // Resonse error message while random factor has hit
                        // or send correct message
                        if (errorMsgPerMsg > 0 && respMsgCnt >= errorMsgPerMsg){
                            if (FPeerReady){
                                LogMsg("Response error message randomly");
                                fillMessageStruct(mRespErrMsg);
                                onSendMessage(mRespErrMsg);
                                respMsgCnt = 0;
                                errorMsgPerMsg = getRandRange(mParam.ErrorFrom, mParam.ErrorTo);
                            }
                        }else{
                            if (FPeerReady){
                                respMsgCnt++;
                                LogMsg("Response message");
                                fillMessageStruct(mRespMsg);
                                onSendMessage(mRespMsg);
                            }
                        }
                        txMsgCnt++;
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
                // and check and response message
                message_t* pmsg = onReceiveMessage();
                if (pmsg != NULL){
                    rxMsgCnt++;
                    memcpy(&receiveMsg, pmsg, sizeof(message_t));
                    if (memcmp(&receiveMsg, &mReqMsg, sizeof(message_t)) == 0){
                        if (errorMsgPerMsg > 0 && respMsgCnt >= errorMsgPerMsg){
                            if (FPeerReady){
                                LogMsg("Response error message");
                                fillMessageStruct(mRespErrMsg);
                                onSendMessage(mRespErrMsg);
                                respMsgCnt = 0;
                                errorMsgPerMsg = getRandRange(mParam.ErrorFrom, mParam.ErrorTo);
                            }
                        }else{
                            if (FPeerReady){
                                LogMsg("Response message");
                                respMsgCnt++;
                                fillMessageStruct(mRespMsg);
                                onSendMessage(mRespMsg);
                            }
                        }
                        txMsgCnt++;
                    }else{
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
bool __fastcall WorkThread::onSendMessage(message_t& msg)
{
    int wpos = 0;
    int headpartlen = offsetof(message_t, content);
    memcpy(sendRawBuff + wpos, &msg.head, headpartlen);
    wpos += headpartlen;
    memcpy(sendRawBuff + wpos, msg.content, msg.clen);
    wpos += msg.clen;
    //CRC16
    msg.crc16 = crc16_cal(sendRawBuff, msg.len - MESSAGE_CRC_LEN - MESSAGE_TAIL_LEN);
    memcpy(sendRawBuff + wpos, &msg.crc16, sizeof(msg.crc16));
    wpos += sizeof(msg.crc16);
    memcpy(sendRawBuff + wpos, &msg.tail, sizeof(msg.tail));
    wpos += sizeof(msg.tail);
    
    sendPos = 0;
    int sendLen = sendData(sendRawBuff, wpos);
    return (sendLen == wpos);
}
//---------------------------------------------------------------------------
message_t* __fastcall WorkThread::onReceiveMessage()
{
    int rvRecv;
    unsigned short rvCRC16;
    unsigned short head;
    unsigned short tail;
    unsigned short msglen;
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
        head = TextToUINT16(mParam.HeadHex);
        if (*(unsigned short*)recvRawBuff == head){
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
        if (rvRecv != MESSAGE_LEN_LEN){
            return NULL;
        }else{
            receivePos = 0;
            mMsgStatus = RECV_MSG_STATUS_DATA;
            msglen = *(unsigned short*)(recvRawBuff + MESSAGE_HEAD_LEN);
        }
        //pass through
    case RECV_MSG_STATUS_DATA:
        rvRecv = receiveData((unsigned char*)&recvRawBuff + MESSAGE_HEAD_LEN + MESSAGE_LEN_LEN,
            msglen - MESSAGE_HEAD_LEN - MESSAGE_LEN_LEN);
        if (rvRecv != msglen - MESSAGE_HEAD_LEN - MESSAGE_LEN_LEN){
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
            mReceiveMsgBuf.len - MESSAGE_CRC_LEN - MESSAGE_TAIL_LEN,
            MESSAGE_CRC_LEN + MESSAGE_TAIL_LEN);
        tail = TextToUINT16(mParam.TailHex);
        if (tail == mReceiveMsgBuf.tail){ // Has a correct message tail
            //Success receive data information and check the message
            rvCRC16 = crc16_cal(recvRawBuff, mReceiveMsgBuf.len - MESSAGE_CRC_LEN - MESSAGE_TAIL_LEN);
            if (rvCRC16 == mReceiveMsgBuf.crc16){
                // Verified success
                bMessageOK = true; // Received a correct message.
            }
        }
        return &mReceiveMsgBuf;

    }
    return NULL;
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::fillMessageStruct(message_t& msg)
{
    //Head
    msg.head = TextToUINT16(mParam.HeadHex);
    //Len
    msg.len = msg.clen + MESSAGE_LEN_EXCEPT_CONTENT;
    // file timestamp
    SYSTEMTIME systm;
    GetLocalTime(&systm);
    msg.timestamp.year = systm.wYear;
    msg.timestamp.mon  = systm.wMonth;
    msg.timestamp.day  = systm.wDay;
    msg.timestamp.hour = systm.wHour;
    msg.timestamp.min  = systm.wMinute;
    msg.timestamp.second = systm.wSecond;
    msg.timestamp.millisec = systm.wMilliseconds;
    
    //Tail
    msg.tail = TextToUINT16(mParam.TailHex);
}
void __fastcall WorkThread::resetParameter(const WorkParameter& param)
{
    mParam = param;
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


