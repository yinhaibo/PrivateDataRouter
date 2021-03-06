//---------------------------------------------------------------------------


#pragma hdrstop

#include "UClientWorkThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


__fastcall ClientWorkThread::ClientWorkThread(device_config_t* pDevCfg)
    : WorkThread(pDevCfg)
{
    receivePos = 0; //Rest receive position to zero

    autoReconnect = true;
    userOpen = false;
    initParameters();

    LogMsg("New client thread.");
}

//---------------------------------------------------------------------------
void __fastcall ClientWorkThread::onStart()
{
    if (!mClient->Active){
        mClient->Active = true;
        userOpen = true;
        LogMsg("Start Event");
    }
}
//---------------------------------------------------------------------------
void __fastcall ClientWorkThread::onStop()
{
    if (mClient->Active){
        mClient->Active = false;
        userOpen = false;
        LogMsg("Stop Event");
    }
}
//---------------------------------------------------------------------------
void __fastcall ClientWorkThread::onParameterChange()
{
    // reinit parameter from param variant
    initParameters();
}
//---------------------------------------------------------------------------
int __fastcall ClientWorkThread::sendData(unsigned char* pbuffer, int len)
{
    if(mClient->Active){
        try{
            int sendLen = 0;
            while(mClient->Socket->Connected && sendLen < len){
                sendLen = pStream->Write(pbuffer + sendPos,
                    len - sendPos);
                if (sendLen > 0){
                    sendPos += sendLen;
                }
            }
            LogMsg("Write :" + IntToStr(sendLen) + ", socket:" + IntToStr((int)mClient->Socket));
            if (sendLen == -1){
                return -1;
            }
            return len;
        }catch(...){
            LogMsg("Socket error in write:" + IntToStr(mClient->Socket->Handle));
            socketErrorProcess();
            return -1;
        }
    }else{
        return 0;
    }
}
//---------------------------------------------------------------------------
// Receive a message from channel, this function can call serveal times
// to get a final result.
// Need to reset receivePos variant to 0 while you need to start a new
// receive procedure.
int __fastcall ClientWorkThread::receiveData(unsigned char* pbuffer, int len)
{
    if (pStream->WaitForData(100)){
        long rdlen = pStream->Read(pbuffer + receivePos,
            len - receivePos);
        //LogMsg("Received :" + IntToStr(rdlen));
        if (rdlen == -1){
            // No data to read
            receivePos = 0;
            return -1;
        }
        receivePos += rdlen;
        if (receivePos == len){
            receivePos = 0;
            return len;
        }else return receivePos;
    }else{
        return 0;
    }
}

//---------------------------------------------------------------------------
//init parameters
void ClientWorkThread::initParameters()
{
    AnsiString ip;
    int port;
    GetTCPClientConfigFromStr(mpDevCfg->configure, ip, port);

    if (mClient == NULL){
        // Create Serial component
        mClient = new TClientSocket(NULL);
        LogMsg("Init connect configure:" + mpDevCfg->configure);
    }
    mClient->Host = ip;
    mClient->Port = port;
    mClient->ClientType = ctBlocking;
    mClient->OnConnect = onSocketConnect;
    mClient->OnDisconnect = onSocketDisconnect;
    mClient->OnError = onSocketError;
}
void __fastcall ClientWorkThread::onSocketConnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Socket connected:" + IntToStr(Socket->Handle));
    pStream = new TWinSocketStream(Socket, 100);
    if (FOnOpenChannel != NULL){
        FOnOpenChannel(this, true);
    }
    FPeerReady = true;
}
void __fastcall ClientWorkThread::onSocketDisconnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Socket disconnected:" + IntToStr(Socket->Handle));
    delete pStream;
    if (mClient != NULL && mClient->Active){
        try{
        mClient->Active = false;
        }catch(...){}
    }
    if (userOpen && autoReconnect){
        //throw new Exception("socket disconnect.");
    }else{
        mIsRunning = false;
        //if (FOnCloseChannel != NULL){
        //    FOnCloseChannel(this, true);
        //}
    }
    FPeerReady = false;
}

void __fastcall ClientWorkThread::onSocketError(System::TObject* Sender,
    TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    LogMsg("Socket error:" + IntToStr(ErrorCode));
    socketErrorProcess();
    ErrorCode = 0;
}

void __fastcall ClientWorkThread::socketErrorProcess()
{
    if (mClient != NULL){
        try{
            if (mClient->Active){
                mClient->Active = false;
            }
        }catch(...){}
        if (userOpen && autoReconnect){
            FStatus = WORK_STATUS_DELAYCONNECT;
        }else{
            //if (FOnCloseChannel != NULL){
            //    FOnCloseChannel(this, true);
            //}
            mIsRunning = false;
        }
        isEnableWrite = false;
        FPeerReady = false;
    }
}

