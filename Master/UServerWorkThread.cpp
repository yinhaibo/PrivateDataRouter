//---------------------------------------------------------------------------


#pragma hdrstop

#include "UServerWorkThread.h"
#include "LogFileEx.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
extern LogFileEx logger;
//---------------------------------------------------------------------------
ServerWorkThread::ServerWorkThread(const device_config_t* pDevCfg,
            const AnsiString& name, Controller* controller)
    : WorkThread(pDevCfg, name, controller)
{
    mServer = NULL;
    receivePos = 0; //Rest receive position to zero
    hasDataRead = false;
    currSocketThread = NULL;

    csBuffer = new TCriticalSection();
    loopbuff_init(&lpbufRx, ucBufRx, LOOPBUFF_MAX_LEN);
    loopbuff_init(&lpbufTx, ucBufTx, LOOPBUFF_MAX_LEN);
}
//---------------------------------------------------------------------------
void __fastcall ServerWorkThread::onInit()
{
    initParameters();
}
//---------------------------------------------------------------------------
// User start current thread
void __fastcall ServerWorkThread::onStart()
{
    LogMsg("Start Event");
    if (mServer->Active){
        // Wait a new connection
    }else{
        try{
            mServer->Active = true;
        }catch(Exception& e){
            LogMsg(FName + " create failured. Detail:" + e.Message);
        }
    }
}
//---------------------------------------------------------------------------
// User stop current thread
void __fastcall ServerWorkThread::onStop()
{
    LogMsg("Stop Event");
    if (mServer->Active){
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
// Do send message
int __fastcall ServerWorkThread::sendData(unsigned char* pbuffer, int len)
{
    int iWriteBytes = 0;
    while(iWriteBytes < len){
        try{
            csBuffer->Enter();
            while(!loopbuff_isfull(&lpbufTx) && iWriteBytes < len){
                loopbuff_push(&lpbufTx, pbuffer[iWriteBytes++]);
            }
        }__finally{
            csBuffer->Leave();
        }
        Sleep(10);
    }
    return iWriteBytes;
}
// Do receive message
int __fastcall ServerWorkThread::receiveData(unsigned char* pbuffer, int len)
{
    int iReadBytes = 0;
    try{
        csBuffer->Enter();
        if (loopbuff_getlen(&lpbufRx, 0) >= (unsigned int)len){
            while(iReadBytes < len){
                pbuffer[iReadBytes++] = loopbuff_pull(&lpbufRx);
            }
        }
    }__finally{
        csBuffer->Leave();
    }
    return iReadBytes;
}
//---------------------------------------------------------------------------
//init parameters
void ServerWorkThread::initParameters()
{
    if (mServer == NULL){
        mServer = new TServerSocket(NULL);
    }
    mServer->Port = StrToInt(mpDevCfg->configure);
    mServer->ThreadCacheSize = 0; //No threads need to cache[Only one client allow]
    mServer->ServerType = stThreadBlocking;
    mServer->OnGetThread = onGetThread;
    mServer->OnClientConnect = onSocketConnect;
    mServer->OnClientDisconnect = onSocketDisconnect;
    mServer->OnClientRead = onSocketRead;
    mServer->OnClientError = onSocketError;
    mServer->OnListen = onServerListen;
    mServer->OnAccept = onServerAccept;
}
__fastcall ServerWorkThread::~ServerWorkThread()
{
    if (mServer != NULL){
        delete mServer;
    }
}
//---------------------------------------------------------------------------
// Client has connected, and need to create a new client work thread
void __fastcall ServerWorkThread::onGetThread(TObject *Sender,
        TServerClientWinSocket *ClientSocket,
        TServerClientThread *&SocketThread)
{
    LogMsg("onGetThread");
    SocketThread = currSocketThread = new ClientWorkThread(ClientSocket,
        FName, &lpbufRx, &lpbufTx, csBuffer);
}
//---------------------------------------------------------------------------
// Client starting to build a new client
void __fastcall ServerWorkThread::onSocketConnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Socket connected:" + Socket->RemoteAddress + ", RemotePort:"
        + IntToStr(Socket->RemotePort));
    isConnected = true;
    StartOK();
}
void __fastcall ServerWorkThread::onSocketDisconnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    // This function will be called by ClientWorkThread
    // So, We just can record the status, do not change any Objects
    LogMsg("Socket disconnected:" + IntToStr(Socket->RemotePort));
    isConnected = false;
    if (mServer != NULL && mServer->Active){
        StopOK();
        currSocketThread = NULL;
    }
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
    LogMsg("Socket error:" + IntToStr(Socket->RemotePort));
    if (mServer != NULL){
        //currSocketThread->Terminate();
        //currSocketThread->WaitFor();
        //delete currSocketThread;
        CloseClientConnection();
        StopOK();
    }
    ErrorCode = 0;
}
void __fastcall ServerWorkThread::onServerListen(TObject *Sender, TCustomWinSocket *Socket)
{
    if (OnServerOpen != NULL){
        OnServerOpen(this, true);
    }
    //FStatus = WORK_STATUS_WORKING;
    LogMsg("Starting to listen");
}
void __fastcall ServerWorkThread::onServerAccept(TObject *Sender, TCustomWinSocket *Socket)
{
    LogMsg("Starting to accept a new connection.");
    CloseClientConnection();
}


void ServerWorkThread::CloseClientConnection()
{
    if (currSocketThread != NULL
        && currSocketThread->ClientSocket != NULL
        && currSocketThread->ClientSocket->Connected){

        try{
            //currSocketThread->ClientSocket->Lock();
            currSocketThread->ClientSocket->Close();
        }__finally{
            //currSocketThread->ClientSocket->Unlock();
        }
        //currSocketThread->WaitFor();
        currSocketThread = NULL;
    }
}
//---------------------------------------------------------------------------



