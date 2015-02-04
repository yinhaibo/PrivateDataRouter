//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <Math.hpp>

#include "UWorkThread.h"
#include "UMsg.h"
#include "LogFileEx.h"
#include "Tools.h"
#include "UAlgorithm.h"

#pragma package(smart_init)

extern LogFileEx logger;
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

__fastcall WorkThread::WorkThread(const device_config_t* pDevCfg,
            IQueue* masterQueue, const AnsiString& name)
    : TThread(true), mpDevCfg(pDevCfg), mMasterQueue(masterQueue), FName(name)
{
    errorMsgPerMsg = -1;
    respMsgCnt = 0;

    lastReportTick = 0;
    txMsgCnt = 0;
    rxMsgCnt = 0;
    errMsgCnt = 0;
    FSeed = 100;

    isConnected = false;
    
    mRawMsgQueue = new RawMsgQueue();

    memset((void*)&mMessage, 0, sizeof(mMessage));
    memset((void*)&mEOFMessage, 0, sizeof(mEOFMessage));
    CREATE_MESSAGE(mMessage, pDevCfg->head, pDevCfg->tag,
        1, pDevCfg->message);
    CREATE_MESSAGE(mEOFMessage, pDevCfg->head, pDevCfg->tag,
        1, pDevCfg->eofMessage);
    /*memset(&mReceiveMsgBuf, 0, sizeof(mReceiveMsgBuf));
    memset(&mSendMsgBuf, 0, sizeof(mSendMsgBuf));*/
    
    mMsgStatus = RECV_MSG_STATUS_HEAD;

    logger.Log("Work Thread [" + IntToStr(this->ThreadID) + "," + FName + "] New.");
}
//---------------------------------------------------------------------------
__fastcall WorkThread::~WorkThread()
{
    if (mRawMsgQueue){
        delete mRawMsgQueue;
    }
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::Start()
{

    try{
        if (FStatus == WORK_STATUS_WAIT){
            FStatus = WORK_STATUS_CONNECT;
        }
    }catch(...){
    }
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::Stop()
{
    //if (FStatus == WORK_STATUS_WORKING){
        FStatus = WORK_STATUS_STOP;
    //}
}
//---------------------------------------------------------------------------
// Do send message
bool __fastcall WorkThread::onSendMessage(RawMsg& msg, int error)
{
    int wpos = 0;
    /*int headpartlen = offsetof(message_t, content);
    memcpy(sendRawBuff + wpos, &msg.head, headpartlen);
    wpos += headpartlen;
    memcpy(sendRawBuff + wpos, msg.content, msg.clen);
    wpos += msg.clen;
    //CRC16
    msg.crc16 = crc16_cal(sendRawBuff, msg.len - MESSAGE_CRC_LEN);
    memcpy(sendRawBuff + wpos, &msg.crc16, sizeof(msg.crc16));
    wpos += sizeof(msg.crc16);
    
    sendPos = 0;
    */

    memcpy(sendRawBuff, msg.stream, msg.len);
    if (error > mpDevCfg->errorThreshold){
        // Write a error message
        sendRawBuff[error % msg.len] = ~sendRawBuff[error % msg.len];
        LogMsg("Write(E):" + StreamToText(sendRawBuff, msg.len));
    }else{
        LogMsg("Write( ):" + StreamToText(sendRawBuff, msg.len));
    }

    int sendLen = sendData(sendRawBuff, msg.len);
    return (sendLen == wpos);
}
//---------------------------------------------------------------------------
// Do receive message
RawMsg* __fastcall WorkThread::onReceiveMessage(int error)
{
    int rvRecv;
    unsigned short rvCRC16;
    unsigned short usDataCRC16;
    unsigned short head;
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
            LogMsg("Error message head.");
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
        usDataCRC16 = *(unsigned short*)(recvRawBuff + mRecvLen - MESSAGE_CRC_LEN);
        //and skip to next step
        receivePos = 0;
        mMsgStatus = RECV_MSG_STATUS_HEAD;

        /*
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
        */
        if (error > mpDevCfg->errorThreshold){
            // read a error message randomly
            recvRawBuff[error % mRecvLen] = ~recvRawBuff[error % mRecvLen];
            LogMsg("Recv(E):" + StreamToText(recvRawBuff, mRecvLen));
        }else{
            LogMsg("Recv( ):" + StreamToText(recvRawBuff, mRecvLen));
        }


        //Success receive data information and check the message
        rvCRC16 = crc16_cal(recvRawBuff, mRecvLen - MESSAGE_CRC_LEN);
        if (rvCRC16 == usDataCRC16){
            // Verified success
            bMessageOK = true; // Received a correct message.
        }

        RawMsg* msg = (RawMsg*)malloc(sizeof(RawMsg));
        memset(msg, 0, sizeof(RawMsg));
        msg->len = mRecvLen;
        memcpy(msg->stream, recvRawBuff, mRecvLen);
        return msg;

    }
    return NULL;
}
//---------------------------------------------------------------------------
void WorkThread::processMessage()
{
    RawMsg* pmsg = onReceiveMessage(getRandRange(1,100));

    if (pmsg != NULL){
        rxMsgCnt++;
        // push a message to master
        Msg* pMasterMsg = new Msg(
                mpDevCfg->source.c_str(),
                mpDevCfg->dest.c_str(),
                pmsg->stream, pmsg->len);
        if (mMasterQueue != NULL){
            //LogMsg("-->push Master Queue");
            mMasterQueue->Push(pMasterMsg);
            LogMsg("Rx, Queue:" + IntToStr(mMasterQueue->Count()));
        }
        delete pmsg;
    }else{
        if (FStatus == WORK_STATUS_CLOSE_WORKING){
            // All data has process finished.
            FStatus = WORK_STATUS_WAIT;
        }
    }

    //Exit when client close connection
    if (FStatus != WORK_STATUS_WORKING) return;
    // send message
    while (!this->Terminated && !mRawMsgQueue->Empty()){
        RawMsg* pmsg = mRawMsgQueue->Pop();
        if (pmsg != NULL){
            txMsgCnt ++;
            LogMsg("Tx, Queue:" + IntToStr(mRawMsgQueue->Count()));
            onSendMessage(*pmsg, getRandRange(1,100));
            delete pmsg;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::Execute()
{
    logger.Log("Work Thread [" + IntToStr(this->ThreadID) + "] start.");
    onInit();
    //---- Place thread code here ----
    while(!this->Terminated){
        switch(FStatus){
        case WORK_STATUS_WAIT:
            Sleep(10);
            break;
        case WORK_STATUS_DELAYCONNECT:  //pass through
        case WORK_STATUS_CONNECT:
            // Start to connect and continue to wait working
            // Starting to work When the connect has builded
            onStart();
            FStatus = WORK_STATUS_WAIT;
            break;
        case WORK_STATUS_WORKING:
        case WORK_STATUS_CLOSE_WORKING:
            processMessage();
            updateUIEvent(txMsgCnt, rxMsgCnt, errMsgCnt, lastReportTick);
            Sleep(10);
            break;
        case WORK_STATUS_STOP:
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
            FStatus = WORK_STATUS_WAIT;
            break;
        default:
            FStatus = WORK_STATUS_WAIT;
        }
    }
    logger.Log("Work Thread [" + IntToStr(this->ThreadID) + "] exit.");
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::updateUIEvent(
    unsigned int& txMsgCnt,
    unsigned int& rxMsgCnt,
    unsigned int& errMsgCnt,
    DWORD& lastReportTick)
{
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
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::resetParameter(device_config_t* pdevCfg)
{
    mpDevCfg = pdevCfg;
    FName = mpDevCfg->source;
}
//---------------------------------------------------------------------------
void WorkThread::LogMsg(AnsiString msg)
{
    logger.Log(FName + "[" + IntToStr(this->ThreadID) + ","
        + IntToStr(GetCurrentThreadId()) + "]\t" + msg);
}
///////////////////////////////////////////
//IRawMsgPush
void WorkThread::Push(RawMsg* pmsg)
{
    if (isConnected){
        if (mpDevCfg->iMaxMsgQueue > 0){
            while(mRawMsgQueue->Count() > mpDevCfg->iMaxMsgQueue){
                RawMsg* cpmsg = mRawMsgQueue->Pop();
                delete cpmsg;
            }
        }
        RawMsg* cpmsg = new RawMsg();
        *cpmsg = *pmsg;
        mRawMsgQueue->Push(cpmsg);
    }else{
        LogMsg("Device has not connected, drop it.");
    }
}

//---------------------------------------------------------------------------
int __fastcall WorkThread::getRandRange(int from , int to)
{
    if (mpDevCfg->errorMode == UNIFORM_DISTRIBUTION){
        return from + 0.5 + ran1(&FSeed) * (to - from);
    }else if(mpDevCfg->errorMode == POISSON_DISTRIBUTION){
        return from + 0.5 + poidev((to - from)/2, &FSeed);
    }else{
        return from;
    }
}


//---------------------------------------------------------------------------
//WorkThreadMessage

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




