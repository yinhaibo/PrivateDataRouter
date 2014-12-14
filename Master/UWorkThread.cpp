//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <Math.hpp>

#include "UWorkThread.h"
#include "UMsg.h"
#include "LogFileEx.h"

#pragma package(smart_init)

extern LogFileEx logger;
//---------------------------------------------------------------------------
WorkMode __fastcall  WorkParameter::fGetMode(void) const
{
    return mMode;
}

WorkParameter::WorkParameter(WorkParameter& value)
{
    this->Mode = value.Mode;
    this->Configure = value.Configure;
    this->Source = value.Source;
    this->Destination = value.Destination;
    this->MasterQueue = value.MasterQueue;
}

WorkParameter WorkParameter::operator=(WorkParameter& value)
{
    this->Mode = value.Mode;
    this->Configure = value.Configure;
    this->Source = value.Source;
    this->Destination = value.Destination;
    this->MasterQueue = value.MasterQueue;

    return *this;
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

void __fastcall WorkParameter::SetSource(AnsiString value)
{
    if(FSource != value) {
        FSource = value;
    }
}
AnsiString __fastcall WorkParameter::GetSource() const
{
    return FSource;
}

void __fastcall WorkParameter::SetDestination(AnsiString value)
{
    if(FDestination != value) {
        FDestination = value;
    }
}
AnsiString __fastcall WorkParameter::GetDestination() const
{
    return FDestination;
}

void __fastcall WorkParameter::SetMasterQueue(IMsgPush* value)
{
    if(FMasterQueue != value) {
        FMasterQueue = value;
    }
}
IMsgPush* __fastcall WorkParameter::GetMasterQueue()
{
    return FMasterQueue;
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

__fastcall WorkThread::WorkThread(WorkParameter& param)
    : TThread(true), mParam(param)
{
    errorMsgPerMsg = -1;
    respMsgCnt = 0;

    mRawMsgQueue = new RawMsgQueue();

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
    mIsRunning = true;
    try{
        onStart();
        this->Resume();
    }catch(...){
    }
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::Stop()
{
    mIsRunning = false;
    try{
        onStop();
        this->Suspend();
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
void __fastcall WorkThread::Execute()
{
    unsigned int lastReportTick = 0;
    unsigned int txMsgCnt = 0;
    unsigned int rxMsgCnt = 0;
    unsigned int errMsgCnt = 0;
    logger.Log("Work Thread [" + IntToStr(this->ThreadID) + "] start.");
    //---- Place thread code here ----
    while(!this->Terminated){
        while (!this->Terminated && mIsRunning){
            // receive message
            RawMsg* pmsg = onReceiveMessage();
            if (pmsg != NULL){
                rxMsgCnt++;
                // push a message to master
                Msg* pMasterMsg = new Msg(mParam.Source.c_str(), mParam.Destination.c_str(), *pmsg);
                if (mParam.MasterQueue != NULL){
                    mParam.MasterQueue->Push(pMasterMsg);
                }
            }
            // send message
            while (!mRawMsgQueue->Empty()){
                mSendMsg = mRawMsgQueue->Pop();
                if (mSendMsg != NULL){
                    txMsgCnt ++;
                    onSendMessage(*mSendMsg);
                    delete mSendMsg;
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
            Sleep(1);
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
    logger.Log("Work Thread [" + IntToStr(this->ThreadID) + "] exit.");
}
//---------------------------------------------------------------------------
void __fastcall WorkThread::resetParameter(WorkParameter& param)
{
    mParam = param;
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
    //
    RawMsg* cpmsg = new RawMsg();
    *cpmsg = *pmsg;
    mRawMsgQueue->Push(cpmsg);
}

//---------------------------------------------------------------------------



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




