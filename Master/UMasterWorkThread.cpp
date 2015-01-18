//---------------------------------------------------------------------------


#pragma hdrstop
#include "Tools.h"
#include <QDialogs.hpp>
#include <stdio.h>
#include "UMasterWorkThread.h"
#include "UServerClientWorkThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

  
extern LogFileEx logger;


void MasterWorkThread::Init()
{
    csVariant = new TCriticalSection();
    FAutoReconnect = true;
    mClientPeer = NULL;

    FStatus = WORK_STATUS_WAIT;
    
    queue = new MsgQueue();
    FMsgHandler = NULL;
    currSocketThread = NULL;

    logger.Log("Master Thread New.");
}

/* Master work thread */
__fastcall MasterWorkThread::MasterWorkThread() : TThread(false)
{
     Init();
}

void __fastcall MasterWorkThread::Open(int port)
{

    FServerMode = true;
    if (mServer == NULL){
        mServer = new TServerSocket(NULL);
    }
    mServer->Port = port;
    
    mServer->ThreadCacheSize = 0; //No threads need to cache[Only one client allow]
    mServer->ServerType = stThreadBlocking;
    mServer->OnGetThread = onGetThread;
    mServer->OnClientConnect = onSocketConnect;
    mServer->OnClientDisconnect = onSocketDisconnect;
    mServer->OnClientRead = onSocketRead;
    mServer->OnClientError = onSocketError;
    mServer->OnListen = onServerListen;
    mServer->OnAccept = onServerAccept;

    try{
        mServer->Active = true;
        FStatus = WORK_STATUS_LISTEN;
    }catch(Exception& e){
        logger.Log(IntToStr(::GetCurrentThreadId()) + "Open failure.");
        FSource = TEXT_MSG_SRC_OP_ERROR;
        FTextMessage = e.Message;
        Synchronize(UpdateTextMessageUI);
        Close();
        FStatus = WORK_STATUS_WAIT;
    }
}

void __fastcall MasterWorkThread::Open(AnsiString ip, int port) 
{
    FServerMode = false;

    if (FMsgHandler == NULL){
        FMsgHandler = new MasterMessageHandler(this);
    }

    if (mClient == NULL){
        // Create Serial component
        mClient = new TClientSocket(NULL);
        logger.Log("Init connect configure:" + ip + ":" + IntToStr(port));
    }
    mClient->Host = ip;
    mClient->Port = port;
    mClient->ClientType = ctBlocking;
    mClient->OnConnect = onSocketConnect;
    mClient->OnDisconnect = onSocketDisconnect;
    mClient->OnRead = onSocketRead;
    mClient->OnWrite = onSocketWrite;
    mClient->OnError = onSocketError;
    try{
        mClient->Active = true;
    }catch(Exception& e){
        logger.Log(IntToStr(::GetCurrentThreadId()) + "Open failure.");
        FSource = TEXT_MSG_SRC_OP_ERROR_CLIENT;
        FTextMessage = e.Message;
        Synchronize(UpdateTextMessageUI);
        Close();
        FStatus = WORK_STATUS_WAIT;
    }
}

void __fastcall MasterWorkThread::Close()
{
    if (FServerMode){
        if (mServer != NULL && mServer->Active){
            try{
                CloseClientConnection();
                mServer->Active = false;

            }catch(...){}
        }
    }else{
        if (mClient != NULL && mClient->Active){
            try{
                mClientPeer->Close();
                mClient->Active = false;
                if (pStream) delete pStream;
                pStream = NULL;
            }catch(...){}
        }
    }
    if (FAutoReconnect){
        FStatus = WORK_STATUS_DELAYCONNECT;
    }else{
        FStatus = WORK_STATUS_WAIT;
    }

    //Reset all receive and send falg variant
    mClientPeer = NULL;
}

void __fastcall MasterWorkThread::InitConnect(int port)
{
    FServerMode = true;
    FIp = "";
    FPort = port;
}

void __fastcall MasterWorkThread::InitConnect(AnsiString ip, int port)
{
    FServerMode = false;
    FIp = ip;
    FPort = port;
}
void __fastcall MasterWorkThread::StartWorking()
{
    if (FStatus == WORK_STATUS_WAIT){
        FStatus = WORK_STATUS_CONNECT;
    }
}

void __fastcall MasterWorkThread::StopWorking()
{
    /*if (FStatus == WORK_STATUS_CONNECT ||
        FStatus == WORK_STATUS_WORKING){
        Close();
    }*/
    FStatus = WORK_STATUS_STOP;
}

__fastcall MasterWorkThread::~MasterWorkThread(void)
{
    Close();
    if (FServerMode){ delete mServer; }
    else {delete mClient;}
    delete csVariant;
    while(!queue->Empty()){
        Msg *pmsg = queue->Pop();
        delete pmsg;
    }
    delete queue;
    delete FMsgHandler;
}

IQueue* MasterWorkThread::GetQueue()
{
    return queue;
}
//---------------------------------------------------------------------------
void __fastcall MasterWorkThread::Execute(void)
{
    AnsiString alias;
    RawMsg msg;
    

    logger.Log("Master Thread Start.");
    while(!this->Terminated){
        switch(FStatus){
        case WORK_STATUS_WAIT:
            Sleep(10);
            break;
        case WORK_STATUS_DELAYCONNECT:
            Sleep(3000);
            FStatus = WORK_STATUS_CONNECT;
            break;
        case WORK_STATUS_CONNECT:
            if (FServerMode){
                Open(FPort);
            }else{
                Open(FIp, FPort);
            }
            // goto working status while connect has builded.
            break;
        case WORK_STATUS_LISTEN:
            if (FServerMode){
            }
            Sleep(10);
            break;
        case WORK_STATUS_WORKING:
            if (FServerMode){
                Sleep(10);
            }else{
                FMsgHandler->ProcessMessage(mClientPeer, pStream);
                Sleep(10);
            }
            break;
        case WORK_STATUS_STOP:
            Close();
            FStatus = WORK_STATUS_WAIT;
            break;
        default:
            Sleep(10);
        }
    }
    logger.Log("Master Thread Exit.");
}

void MasterWorkThread::dispatchMsg(Msg* pmsg)
{
    if (csWorkVar != NULL){
        try{
            csWorkVar->Enter();
            dispatchMsgSafe(pmsg);
        }__finally{
            csWorkVar->Leave();
        }
    }else{
        dispatchMsgSafe(pmsg);
    }
}

void MasterWorkThread::dispatchMsgSafe(Msg* pmsg)
{
    AnsiString dest = AnsiString(pmsg->to);
    int idx = FThreadList->IndexOf(dest);
    if (idx == -1){
        logger.Log("Target " + dest + " has not found in configure list.");
    }else{
        WorkThread* thread = (WorkThread*)FThreadList->Objects[idx];
        thread->Push(&pmsg->rawmsg);
        //logger.Log("Push a message to " + dest);
    }
}

void MasterWorkThread::LogRxMsg(Msg* pmsg)
{
    static char buffer[100];
    snprintf(buffer, 100, "[Master]Rx:[%s->%s] CL:%d", pmsg->from
        , pmsg->to, (pmsg->rawmsg.clen));
    logger.Log(buffer);
}

void MasterWorkThread::LogTxMsg(Msg* pmsg)
{
    static char buffer[100];
    snprintf(buffer, 100, "[Master]Tx:[%s->%s] CL:%d", pmsg->from
        , pmsg->to, (pmsg->rawmsg.clen));
    logger.Log(buffer);
}

//---------------------------------------------------------------------------
void __fastcall MasterWorkThread::onSocketConnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Master connected:"
        + Socket->RemoteAddress + ":" + IntToStr(Socket->RemotePort)
        + "[" + IntToStr(::GetCurrentThreadId()) + "]");

    if (FOnOpenChannel != NULL){
        FOnOpenChannel(NULL, true);
    }
    if (!FServerMode){
        mClientPeer = Socket;
        pStream = new TWinSocketStream(mClientPeer, 100);
    }
    FStatus = WORK_STATUS_WORKING;
}
void __fastcall MasterWorkThread::onSocketDisconnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Master disconnected:" + Socket->RemoteAddress
            + ":" + IntToStr(Socket->RemotePort)
            + "[" + IntToStr(::GetCurrentThreadId()) + "]");

    if (FServerMode){
        currSocketThread = NULL;
        if (FOnCloseChannel != NULL){
            FOnCloseChannel(NULL, true);
        }
    }else{
        if (FAutoReconnect){
            FStatus = WORK_STATUS_DELAYCONNECT;
            FSource = TEXT_MSG_SRC_CLIENT_NOTIFY;
            FTextMessage = "Wait to reconnect...";
            Synchronize(UpdateTextMessageUI);
        }else{
            FStatus = WORK_STATUS_WAIT;
            if (FOnCloseChannel != NULL){
                FOnCloseChannel(NULL, true);
            }
        }   
    }
}
void __fastcall MasterWorkThread::onSocketRead(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    logger.Log("Master onSocketRead:" + IntToStr(::GetCurrentThreadId()));
}
void __fastcall MasterWorkThread::onSocketWrite(System::TObject* Sender,
    TCustomWinSocket* Socket)
{

}

void __fastcall MasterWorkThread::onSocketError(System::TObject* Sender,
    TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    logger.Log(AnsiString("Socket error:") + IntToStr(Socket->Handle));
    if (FServerMode && mServer != NULL){
        Socket->Close();
        if (mClientPeer == Socket){
            mClientPeer = NULL;
        }
    }else{
        if (FOnCloseChannel != NULL){
            FOnCloseChannel(NULL, true);
        }
    }
    CloseClientConnection();
    ErrorCode = 0;
}
void __fastcall MasterWorkThread::onServerListen(TObject *Sender, TCustomWinSocket *Socket)
{
    if (OnServerOpen != NULL){
        OnServerOpen(NULL, true);
    }
    LogMsg("Master server port has opened.");
}
//---------------------------------------------------------------------------
void MasterWorkThread::LogMsg(AnsiString msg)
{
    logger.Log("[Master]\t" + msg);
}
void __fastcall MasterWorkThread::onServerAccept(TObject *Sender, TCustomWinSocket *Socket)
{
    LogMsg(AnsiString("Starting to accept a new connection.")
        + "[" + IntToStr(::GetCurrentThreadId()) + "]");
    CloseClientConnection();

    //Notify UI
    FSource = TEXT_MSG_SRC_CLIENT_INFO;
    FTextMessage = Socket->RemoteHost;
    Synchronize(UpdateTextMessageUI);
}

void __fastcall MasterWorkThread::UpdateTextMessageUI(void)
{
    if (FOnTextMessage){
        FOnTextMessage(FSource, FTextMessage);
    }
}

void MasterWorkThread::CloseClientConnection()
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
void __fastcall MasterWorkThread::onGetThread(System::TObject* Sender,
            TServerClientWinSocket* ClientSocket,
            TServerClientThread* &SocketThread)
{
    LogMsg(AnsiString("Master create client work thread.")
        + "[" + IntToStr(::GetCurrentThreadId()) + "]");
    SocketThread = currSocketThread = new ServerClientWorkThread(ClientSocket, this);
}
//---------------------------------------------------------------------------




