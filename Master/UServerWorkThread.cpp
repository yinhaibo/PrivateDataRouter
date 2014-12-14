//---------------------------------------------------------------------------


#pragma hdrstop

#include "UServerWorkThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
ServerWorkThread::ServerWorkThread(WorkParameter& param)
    : WorkThread(param)
{
    mServer = NULL;
    mClient = NULL;
    receivePos = 0; //Rest receive position to zero
    hasDataRead = false;
    initParameters();
}
//---------------------------------------------------------------------------
void __fastcall ServerWorkThread::onStart()
{
    LogMsg("Start Event");
    if (!mServer->Active){
        if (mClient != NULL){
            mClient->Close();
            mClient = NULL;
            hasDataRead = false;
        }
        mServer->Active = true;
    }
}
//---------------------------------------------------------------------------
void __fastcall ServerWorkThread::onStop()
{
    LogMsg("Stop Event");
    if (mServer->Active){
        if (mClient != NULL){
            mClient->Close();
            mClient = NULL;
            hasDataRead = false;
        }
        mServer->Active = false;
    }
}
//---------------------------------------------------------------------------
void __fastcall ServerWorkThread::onParameterChange()
{
    // reinit parameter from param variant
    initParameters();
}
//---------------------------------------------------------------------------
void __fastcall ServerWorkThread::onSendMessage(RawMsg& msg)
{
    if(mServer->Active && mClient != NULL){
        mClient->SendBuf(msg.message, 5);
    }
}
RawMsg* __fastcall ServerWorkThread::onReceiveMessage()
{
    if(mServer->Active && mClient != NULL/* && hasDataRead*/){
        long rdlen = mClient->ReceiveBuf(mReceiveMsgBuf.message + receivePos,
            MESSAGE_LEN - receivePos);
        if (rdlen == -1){
            // No data to read
            hasDataRead = false;
            receivePos = 0; // Reset receive position, will drop some bytes.
            return NULL;
        }
        receivePos += rdlen;
        if (receivePos == MESSAGE_LEN){
            receivePos = 0;
            return &mReceiveMsgBuf;
        }else return NULL;
    }else return NULL;
}
//---------------------------------------------------------------------------
//init parameters
void ServerWorkThread::initParameters()
{
    if (mServer == NULL){
        mServer = new TServerSocket(NULL);
    }
    mServer->Port = StrToInt(mParam.Configure);
    mServer->ServerType = stNonBlocking;
    mServer->OnClientConnect = onSocketConnect;
    mServer->OnClientDisconnect = onSocketDisconnect;
    mServer->OnClientRead = onSocketRead;
    mServer->OnClientError = onSocketError;
    mServer->OnListen = onServerListen;
}
__fastcall ServerWorkThread::~ServerWorkThread()
{
    if (mServer != NULL){
        delete mServer;
    }
}
//---------------------------------------------------------------------------
void __fastcall ServerWorkThread::onSocketConnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Socket connected:" + Socket->RemoteAddress + "Handle:" + IntToStr(Socket->Handle));
    if (mClient != NULL){
        mClient->Close(); // close prev connection
    }
    mClient = Socket;
    if (FOnOpenChannel != NULL){
        FOnOpenChannel(this, true);
    }
    mIsRunning = true;
}
void __fastcall ServerWorkThread::onSocketDisconnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Socket disconnected:" + IntToStr(Socket->Handle));
    if (mServer != NULL){
        if (mClient == Socket){
            mClient = NULL;
        }
        if (FOnCloseChannel != NULL){
            FOnCloseChannel(this, true);
        }
    }
    mIsRunning = false;
}
void __fastcall ServerWorkThread::onSocketRead(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    //LogMsg("Read event.");
    if (mServer != NULL){
        hasDataRead = true;
    }
}
void __fastcall ServerWorkThread::onSocketError(System::TObject* Sender,
    TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    LogMsg("Socket error:" + IntToStr(Socket->Handle));
    if (mServer != NULL){
        Socket->Close();
        if (mClient == Socket){
            mClient = NULL;
        }
        if (FOnCloseChannel != NULL){
            FOnCloseChannel(this, true);
        }
    }
    ErrorCode = 0;
}
void __fastcall ServerWorkThread::onServerListen(TObject *Sender, TCustomWinSocket *Socket)
{
    if (OnServerOpen != NULL){
        OnServerOpen(this, true);
    }
    mIsRunning = true;
}
//---------------------------------------------------------------------------

