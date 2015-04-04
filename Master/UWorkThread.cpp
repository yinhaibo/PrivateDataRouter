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

__fastcall WorkThread::WorkThread(device_config_t* pDevCfg,
            const AnsiString& name, Controller* controller)
    : TThread(true), mpDevCfg(pDevCfg), FName(name), FController(controller)
{
    errorMsgPerMsg = -1;
    FPrevMsgSeq = 0;
    FPrevMsgCRC = 0;
    respMsgCnt = 0;

    lastReportTick = 0;
    txMsgCnt = 0;
    rxMsgCnt = 0;
    errMsgCnt = 0;
    FSeed = 100;
    FReSendInterval = 1000; //1000ms = 1s

    isConnected = false;
    FLocalMessage = false;
    FPrevMsg = NULL;

    //Create channel from controller (No priority)
    AnsiString alias = pDevCfg->source;
    // Inner channel have maxinum priority
    FChannel = FController->registerChannel(alias, this, 0x7FFFFFFF);
    
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

    char buff[50];
    snprintf(buff, 50, "Work Thread [%d,%s] New.",
        this->ThreadID, FName.c_str());
    logger.Log(buff);
}
//---------------------------------------------------------------------------
__fastcall WorkThread::~WorkThread()
{
    if (mRawMsgQueue){
        while (mRawMsgQueue->Count() > 0){
            RawMsg* rawmsg = mRawMsgQueue->Pop();
            delete rawmsg;
        }
        delete mRawMsgQueue;
    }
    delete FPrevMsg;
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
void __fastcall WorkThread::StartOK()
{
    if (FOnOpenChannel != NULL){
        FOnOpenChannel(this, true);
    }
    FStatus = WORK_STATUS_WORKING;
    if (FChannel != NULL){
        FChannel->Open();
    }
}
void __fastcall WorkThread::StopOK()
{
    if (FOnCloseChannel != NULL){
        FOnCloseChannel(this, true);
    }
    FStatus = WORK_STATUS_CLOSE_WORKING; // Wait for another client
        
    if (FChannel != NULL){
        FChannel->Close();
    }
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
    /*if (error > mpDevCfg->errorThreshold){
        // Write a error message
        sendRawBuff[error % msg.len] = ~sendRawBuff[error % msg.len];
        LogMsg("Write(E):" + StreamToText(sendRawBuff, msg.len));
    }else{
        LogMsg("Write( ):" + StreamToText(sendRawBuff, msg.len));
    }*/
    LogMsg("Write:" + StreamToText(sendRawBuff, msg.len));
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
        /*if (error > mpDevCfg->errorThreshold){
            // read a error message randomly
            recvRawBuff[error % mRecvLen] = ~recvRawBuff[error % mRecvLen];
            LogMsg("Recv(E):" + StreamToText(recvRawBuff, mRecvLen));
        }else{
            LogMsg("Recv( ):" + StreamToText(recvRawBuff, mRecvLen));
        }*/

        LogMsg("Recv:" + StreamToText(recvRawBuff, mRecvLen));
        //Success receive data information and check the message
        rvCRC16 = crc16_cal(recvRawBuff, mRecvLen - MESSAGE_CRC_LEN);
        if (rvCRC16 == usDataCRC16){
            // Verified success
            bMessageOK = true; // Received a correct message.

            if (((message_t*)recvRawBuff)->tag == mMessage.tag){
                FLocalMessage = true;
            }else{
                FLocalMessage = false;
            }
            FCurrentMsgSeq = ((message_t*)recvRawBuff)->seq;
            FCurrentMsgCRC = usDataCRC16;
        }

        RawMsg* msg = new RawMsg();//(RawMsg*)malloc(sizeof(RawMsg));
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
    char buff[40];
    /////////////////////////////
    // Receive message from device
    RawMsg* prawmsg = onReceiveMessage(getRandRange(1,100));

    if (prawmsg != NULL){
        rxMsgCnt++;
        if (bMessageOK){
            // Receive and make a copy for device message
            if (FLocalMessage
                && !(FCurrentMsgSeq == FPrevMsgSeq && FCurrentMsgCRC == FPrevMsgCRC)){
                FPrevMsgSeq = FCurrentMsgSeq;
                FPrevMsgCRC = FCurrentMsgCRC;
                if (FPrevMsg != NULL){
                    snprintf(buff, 40, "%s--> delete copy Msg:%08ul",
                        FName.c_str(), FPrevMsg->msgid);
                    logger.Log(buff);
                    delete FPrevMsg;
                }
                LogMsg(*prawmsg, "Receive a new local message, buffer and push to master");
                if (FController != NULL){
                    // push a message to master
                    Msg* pmsg = new Msg(
                            mpDevCfg->source.c_str(),
                            mpDevCfg->dest.c_str(),
                            prawmsg->stream, prawmsg->len);

                    sendLocalMessage(pmsg);
                    
                    snprintf(buff, 40, "%s--> new Msg:%08ul", FName.c_str(), pmsg->msgid);
                    logger.Log(buff);
                    FPrevMsg = new Msg(*pmsg);
                    snprintf(buff, 40, "%s--> new bak Msg:%08ul", FName.c_str(),  FPrevMsg->msgid);
                    logger.Log(buff);
                    LogMsg("dispatch message to master");
                    FDispatchChannel = FController->dispatchMsg(FChannel, pmsg);
                    FMsgSendTick = ::GetTickCount();
                }
            }else{
                // The device send a retransmillion(see Local Message Flag)
                // or response message, the device proxy send to master
                LogMsg(*prawmsg, "Receive a new message, push to master");
                //duplicate message, push to master
                if (FController != NULL){
                    Msg* pmsg = new Msg(
                        mpDevCfg->source.c_str(),
                        mpDevCfg->dest.c_str(),
                        prawmsg->stream, prawmsg->len);

                    snprintf(buff, 40, "%s-->> new Msg:%08ul", FName.c_str(), pmsg->msgid);
                    logger.Log(buff);
                    if (FLocalMessage){
                        sendLocalMessage(pmsg);
                        
                        FController->decChannelPriority(FDispatchChannel);
                        FDispatchChannel = FController->dispatchMsg(FChannel, pmsg, FDispatchChannel);
                        // Only reset message send tick in retransmission local message
                        FMsgSendTick = ::GetTickCount();
                    }else{
                        FController->dispatchMsg(FChannel, pmsg, FDispatchChannel);
                    }
                }
            }
        }else{
            // error message, just wait a normal message

        }
        delete prawmsg;
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
            //LogMsg("Tx, Queue:" + IntToStr(mRawMsgQueue->Count()));
            LogMsg(*pmsg, "push to simulator");
            onSendMessage(*pmsg, getRandRange(1,100));
            delete pmsg;
        }
    }

    // Resend message in resend interval milliseconds
    if (FPrevMsg != NULL &&
        ::GetTickCount() - FMsgSendTick > FReSendInterval){
        LogMsg(FPrevMsg->rawmsg, "retransmission message to master");
        Msg* pmsg = new Msg(*FPrevMsg);
        sendLocalMessage(pmsg);
        FController->decChannelPriority(FDispatchChannel);
        FDispatchChannel = FController->dispatchMsg(FChannel, pmsg, FDispatchChannel);
        FMsgSendTick = ::GetTickCount();
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
    char buffer[300];
    snprintf(buffer, 300, "%s[%u]\t%s", FName.c_str(),GetCurrentThreadId(), msg.c_str());
    logger.Log(buffer);
}
//---------------------------------------------------------------------------
void WorkThread::LogMsg(RawMsg& msg, AnsiString text)
{
    char buffer[200];
    message_t* pmsgstruct = (message_t*)msg.stream;
    unsigned short crc = *(unsigned short*)(msg.stream + msg.len - 2);
    snprintf(buffer, 200, "[%s]Tag:%u, Seq:%u, CRC:%04x -- %s", FName.c_str(),
        pmsgstruct->tag, pmsgstruct->seq, crc,
        text.c_str());
    logger.Log(buffer);
}
///////////////////////////////////////////
//IRawMsgPush
void WorkThread::Push(Msg* pmsg)
{
    if (isConnected){
        // valid message
        if (pmsg->validedOK){
            if (mpDevCfg->iMaxMsgQueue > 0){
                while(mRawMsgQueue->Count() > mpDevCfg->iMaxMsgQueue){
                    RawMsg* cpmsg = mRawMsgQueue->Pop();
                    delete cpmsg;
                }
            }
            RawMsg* rawmsg = new RawMsg(pmsg->rawmsg);
            mRawMsgQueue->Push(rawmsg);

            char buff[40];
            
            // check message is send success or not
            if (FPrevMsg != NULL &&
                pmsg->rawmsg.len == FPrevMsg->rawmsg.len){
                if (memcmp(pmsg->rawmsg.stream,
                    FPrevMsg->rawmsg.stream,
                    pmsg->rawmsg.len) == 0){

                    successTransMessage(FPrevMsg);

                    if (FDispatchChannel != NULL){
                        FController->incChannelPriority(FDispatchChannel);
                        FDispatchChannel = NULL;
                    }
                    snprintf(buff, 40, "success sent, delete Msg:%08ul", FPrevMsg->msgid);
                    logger.Log(buff);

                    // the message was sent success
                    delete FPrevMsg;
                    FPrevMsg = NULL;
                }
            }
            snprintf(buff, 40, "delete Msg:%08ul", pmsg->msgid);
            logger.Log(buff);
            delete pmsg;
        }else{
            if (strcmp(pmsg->from, FPrevMsg->from) == 0){
                // these is a error back message
                // resend current message immediately
                if (FController != NULL){
                    LogMsg("resend current message because of error back");
                    FController->dispatchMsg(FChannel, FPrevMsg);
                }
            }else{
                // these is a error message from source channel
                // just back to the originator
                if (FController != NULL){
                    LogMsg("resend origne message because of error CRC");
                    // exchange from and to field and recalculate CRC field

                    FController->dispatchMsg(FChannel, pmsg);
                }
            }
        }
        mpDevCfg->msgRxCnt++;
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

void WorkThread::reinitMessageCntVariant()
{
    mpDevCfg->sendSeq = 0;
    mpDevCfg->msgMsgSent = 0;
    mpDevCfg->msgTxCnt = 0;
    mpDevCfg->msgRxCnt = 0;
    mpDevCfg->msgErrCnt = 0;
    mpDevCfg->sendTick = 0;
        
    mpDevCfg->dcResendCnt.val = 0;
    mpDevCfg->dcResendCnt.avg = 0.0f;
    mpDevCfg->dcResendCnt.min = MAX_COUNT_VALUE;
    mpDevCfg->dcResendCnt.max = MIN_COUNT_VALUE;
        
    mpDevCfg->dcRespTime.val = 0;
    mpDevCfg->dcRespTime.avg = 0.0f;
    mpDevCfg->dcRespTime.min = MAX_COUNT_VALUE;
    mpDevCfg->dcRespTime.max = MIN_COUNT_VALUE;

    for (int i = 0; i < MAX_RETRANS_REC_CNT; i++){
        mpDevCfg->resendTotal[i] = 0;
    }
}

void WorkThread::sendLocalMessage(Msg* pmsg)
{
    if (mpDevCfg->sendSeq == 0){
        // First time
        reinitMessageCntVariant();
        mpDevCfg->msgMsgSent = 1;
        mpDevCfg->resendTotal[0] = 1;
        mpDevCfg->sendSeq = FCurrentMsgSeq;
        mpDevCfg->sendCRC = FCurrentMsgCRC;
        LogMsg(pmsg->rawmsg, "Message sent:" + IntToStr(mpDevCfg->msgMsgSent));
    }else if (mpDevCfg->sendSeq == FCurrentMsgSeq &&
              mpDevCfg->sendCRC == FCurrentMsgCRC){
        mpDevCfg->dcResendCnt.val++;
        LogMsg(pmsg->rawmsg, " send Times:" +
            IntToStr(mpDevCfg->dcResendCnt.val));
    }else{
        // Have a new message, the prev message has sent
        // Calc min, max and avg
        mpDevCfg->msgMsgSent++;
        if (mpDevCfg->dcResendCnt.val >= MAX_RETRANS_REC_CNT){
            mpDevCfg->resendTotal[MAX_RETRANS_REC_CNT-1]++;
        }else{
            mpDevCfg->resendTotal[mpDevCfg->dcResendCnt.val]++;
        }
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
        mpDevCfg->sendSeq = FCurrentMsgSeq;  // Update send sequence
        mpDevCfg->sendCRC = FCurrentMsgCRC;
        mpDevCfg->dcResendCnt.val = 0;// Reset resend value
        LogMsg(pmsg->rawmsg, "Message sent:" + IntToStr(mpDevCfg->msgMsgSent));
    }
    mpDevCfg->sendTick = GetTickCount();
    mpDevCfg->msgTxCnt++;
}
void WorkThread::successTransMessage(Msg* pmsg)
{
    //calc response time
    mpDevCfg->dcRespTime.val = GetTickCount() - mpDevCfg->sendTick; 
    mpDevCfg->dcRespTime.avg =
        ((mpDevCfg->msgMsgSent - 1.0f) * mpDevCfg->dcRespTime.avg
            + mpDevCfg->dcRespTime.val)
        / mpDevCfg->msgMsgSent;
    if (mpDevCfg->dcRespTime.min > mpDevCfg->dcRespTime.val){
        mpDevCfg->dcRespTime.min = mpDevCfg->dcRespTime.val;
    }
    if (mpDevCfg->dcRespTime.max < mpDevCfg->dcRespTime.val){
        mpDevCfg->dcRespTime.max = mpDevCfg->dcRespTime.val;
    }
}


