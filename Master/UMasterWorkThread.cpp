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

    FStartWorkingMode = false;


    FStatus = WORK_STATUS_WAIT;
    
    //queue = new MsgQueue(); // Only for client mode
    //FChannel = FController->registerChannel("", queue, 0);
    FMsgHandler = NULL;
    currSocketThread = NULL;

    logger.Log("Master Thread New.");
}

/* Master work thread */
__fastcall MasterWorkThread::MasterWorkThread(int ch, Controller* controller,
    int priority)
    : TThread(false), FCh(ch), FController(controller),FPriority(priority)
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
        FMsgHandler = new MasterMessageHandler(this, NULL, FController);
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
        FSource = TEXT_MSG_SRC_CLIENT_NOTIFY;
        FTextMessage = e.Message;
        Synchronize(UpdateTextMessageUI);
        Close();
        //FStatus = WORK_STATUS_DELAYCONNECT;
    }
}

void __fastcall MasterWorkThread::Close()
{
    if (FServerMode){
        if (mServer != NULL && mServer->Active){
            try{
                LogMsg("Close all client thread...");
                // Close all client connections
                map<DWORD, ServerClientWorkThread*>::iterator it;
                ServerClientWorkThread* wt;
                for (it = mapClientThread.begin();
                     it != mapClientThread.end();
                     ++it){
                     wt = it->second;
                     wt->Terminate();
                     wt->WaitFor();
                     it->second = NULL;
                     delete wt;
                }

                mServer->Active = false;
                mapClientThread.clear();

            }catch(Exception& e){
                LogMsg("Close client error:" + e.Message);
            }
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
    if (FAutoReconnect && FStartWorkingMode){
        FStatus = WORK_STATUS_DELAYCONNECT;
        FSource = TEXT_MSG_SRC_CLIENT_NOTIFY;
        FTextMessage = "Ready to reconnect to master...";
        Synchronize(UpdateTextMessageUI);
    }else{
        FStatus = WORK_STATUS_WAIT;
        FSource = TEXT_MSG_SRC_CLIENT_NOTIFY;
        FTextMessage = "Ready to start...";
        Synchronize(UpdateTextMessageUI);
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
    FStartWorkingMode = true;
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
    FStartWorkingMode = false;
    FStatus = WORK_STATUS_STOP;
}

__fastcall MasterWorkThread::~MasterWorkThread(void)
{
    Close();
    if (FServerMode){ delete mServer; }
    else {delete mClient;}
    delete csVariant;
    /*while(!queue->Empty()){
        Msg *pmsg = queue->Pop();
        delete pmsg;
    }
    delete queue; */
    delete FMsgHandler;
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
/*
void MasterWorkThread::dispatchMsg(Msg* pmsg)
{
    dispatchMsg(NULL, pmsg);
}
void MasterWorkThread::dispatchMsg(ServerClientWorkThread* client, Msg* pmsg)
{
    if (csWorkVar != NULL){
        try{
            csWorkVar->Enter();
            dispatchMsgSafe(client, pmsg);
        }__finally{
            csWorkVar->Leave();
        }
    }else{
        dispatchMsgSafe(client, pmsg);
    }
}

void MasterWorkThread::dispatchMsgSafe(ServerClientWorkThread* client, Msg* pmsg)
{
    AnsiString dest = AnsiString(pmsg->to);
    if (FServerMode){
        if (pmsg->msgtype == MSGTYPE_DATA){
            // Find dest location and dispatch them
            // At first, check inner alias
            if (DispatchToInner(dest, &pmsg->rawmsg)){
                delete pmsg;
                return;
            }
            // then check outer alias list in client thread list
            ServerClientWorkThread* thread;
            map<DWORD, ServerClientWorkThread*>::iterator it;
            for (it = mapClientThread.begin();
                 it != mapClientThread.end();
                 ++it){
                 thread = it->second;
                 if (thread != NULL){
                    // allow one message send to mulitple channel
                    if (thread->IsBelong(dest)){
                        thread->Push(pmsg);
                    }
                 }
            }
        }else if (pmsg->msgtype == MSGTYPE_TAGLIST){
            // Bind a new dest location set.
            client->UpdateTagList(pmsg);
        }
    }else{
        // Receive from other master, dispatch to inner channel
        if (DispatchToInner(dest, &pmsg->rawmsg)){
            delete pmsg;
        }else{
            // Receive from inner channel dispatch to other master
            queue->Push(pmsg);
        }
    }
}

bool MasterWorkThread::DispatchToInner(AnsiString& dest, RawMsg* rawmsg)
{
    int idx = FThreadList->IndexOf(dest);
    if (idx == -1){
        return false;
    }else{
        WorkThread* thread = (WorkThread*)FThreadList->Objects[idx];
        thread->Push(rawmsg);
        return true;
    }
}
*/
void MasterWorkThread::LogRxMsg(Msg* pmsg)
{
    LogMsg(pmsg, "Rx");
}

void MasterWorkThread::LogTxMsg(Msg* pmsg)
{
    LogMsg(pmsg, "Tx");
}

void MasterWorkThread::LogMsg(Msg* pmsg, char* tag)
{
    static char buffer[100];
    if (pmsg == NULL){
        snprintf(buffer, 100, "[Master]Tx:[NULL]");
    }else{
        if (pmsg->msgtype == MSGTYPE_DATA){
            snprintf(buffer, 100, "[Master]%s:[%s->%s] L:%d", tag, pmsg->from
                , pmsg->to, (pmsg->rawmsg.len));
        }else if(pmsg->msgtype == MSGTYPE_TAGLIST){
            memset(buffer, 0, sizeof(buffer));
            snprintf(buffer, 100, "[Master]%s:TAG LIST:", tag);
            for (int i = 0; i < MAX_ALIAS_CNT; i++){
                if (pmsg->taglist[i][0] == '\0') break;
                strcpy(buffer + strlen(buffer), pmsg->taglist[i]);
                strcpy(buffer + strlen(buffer), ",");
                if (strlen(buffer) > 100 - ALIAS_LEN){
                    break;
                }
            }
        }else{
            snprintf(buffer, 100, "[Master]%s:[UNKNOWN] L:%d", tag, pmsg->from
                , pmsg->to, (pmsg->rawmsg.len));
        }
    }
    logger.Log(buffer);
}

void MasterWorkThread::LogMsgStream(AnsiString& stream)
{
    logger.Log(stream.c_str());
}

bool __fastcall MasterWorkThread::isTerminated()
{
    return this->Terminated;
}

//---------------------------------------------------------------------------
void __fastcall MasterWorkThread::onSocketConnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Master connected:"
        + Socket->RemoteAddress + ":" + IntToStr(Socket->RemotePort)
        + "[" + IntToStr(::GetCurrentThreadId()) + "]");


    if (FOnOpenChannel != NULL){
        FOnOpenChannel(FCh, true);
    }
    if (!FServerMode){
        mClientPeer = Socket;
        pStream = new TWinSocketStream(mClientPeer, 100);

        FMsgHandler->getChannel()->setAlias("*");
        FMsgHandler->getChannel()->Open();
        // Add tag list msg to queue on connection has build
        Msg* msg = new Msg();
        BuildTagListMsg(msg);

        bool succflag = FController->dispatchMsg(FMsgHandler->getChannel(), msg);
        if (succflag){
            LogMsg("Send channel tag list success.");
        }else{
            LogMsg("Send channel tag list failed.");
        }
    }

    FStatus = WORK_STATUS_WORKING;
}

void MasterWorkThread::BuildTagListMsg(Msg* msg)
{
    if (FThreadList != NULL && FThreadList->Count > 0){
        for (int i = 0; i < FThreadList->Count; i++){
            strcpy(msg->taglist[i], FThreadList->Strings[i].c_str());
        }
    }
}

void __fastcall MasterWorkThread::onSocketDisconnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    LogMsg("Master disconnected:" + Socket->RemoteAddress
            + ":" + IntToStr(Socket->RemotePort)
            + "[" + IntToStr(::GetCurrentThreadId()) + "]");

    
    if (FServerMode){
        DWORD threadid = ::GetCurrentThreadId();
        map<DWORD, ServerClientWorkThread*>::iterator it;
        it = mapClientThread.find(threadid);
        if (it != mapClientThread.end()){
            mapClientThread.erase(it);
            currSocketThread = NULL;
        }else{
            LogMsg("Exception: No thread id found.");
        }
        UpdateConnectStatus();
    }else{
        if (FStartWorkingMode && FAutoReconnect){
            FStatus = WORK_STATUS_DELAYCONNECT;
            FSource = TEXT_MSG_SRC_CLIENT_NOTIFY;
            FTextMessage = "Wait to reconnect...";
            Synchronize(UpdateTextMessageUI);
            UpdateConnectStatus();
        }else{
            FStatus = WORK_STATUS_WAIT;
            if (FOnCloseChannel != NULL){
                FOnCloseChannel(FCh, true);
            }
        }      
        if (pStream != NULL){
            delete pStream;
            pStream = NULL;
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
            FOnCloseChannel(FCh, true);
        }
    }
    CloseClientConnection();
    ErrorCode = 0;
}
void __fastcall MasterWorkThread::onServerListen(TObject *Sender, TCustomWinSocket *Socket)
{
    if (OnServerOpen != NULL){
        OnServerOpen(FCh, true);
    }
    LogMsg("Master server port has opened.");
}
//---------------------------------------------------------------------------
void MasterWorkThread::LogMsg(AnsiString msg)
{
    logger.Log("[Master-" + IntToStr(FCh) + "," + IntToStr(::GetCurrentThreadId()) + "]\t" + msg);
}
void __fastcall MasterWorkThread::onServerAccept(TObject *Sender, TCustomWinSocket *Socket)
{
    LogMsg(AnsiString("Starting to accept a new connection.")
        + "[" + IntToStr(::GetCurrentThreadId()) + "]");
    //CloseClientConnection();

    
}

void __fastcall MasterWorkThread::UpdateTextMessageUI(void)
{
    if (FOnTextMessage){
        FOnTextMessage(FCh, FSource, FTextMessage);
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
    SocketThread = currSocketThread = new ServerClientWorkThread(this,
        ClientSocket, FController);
    mapClientThread[SocketThread->ThreadID]  = currSocketThread;

    //Notify UI
    UpdateConnectStatus();
}
//---------------------------------------------------------------------------
void __fastcall MasterWorkThread::UpdateConnectStatus()
{
    //if (mapClientThread.size() > 0){
        FSource = TEXT_MSG_SRC_CLIENT_COUNT;
        FTextMessage = IntToStr(mapClientThread.size());
        Synchronize(UpdateTextMessageUI);
    //}else{
    //}
}
//---------------------------------------------------------------------------
void MasterWorkThread::Push(Msg* pmsg)
{
    /*if (queue != NULL){
        queue->Push(pmsg);
    }*/
}

int MasterWorkThread::GetChannelPriority()
{
    if (FServerMode){
        if (mapClientThread.size() == 0) return 0;
        int totalPri = 0;
        map<DWORD, ServerClientWorkThread*>::iterator it;
        for (it = mapClientThread.begin();
             it != mapClientThread.end();
             ++it){
             totalPri += it->second->GetChannelPriority();
        }
        return totalPri / mapClientThread.size();
    }else{
        return FMsgHandler->getChannel()->getPriority();
    }
}

void MasterWorkThread::UdateChannelErrorMode(const master_config_t* config)
{
    if (FServerMode){
        if (mapClientThread.size() == 0) return;
        map<DWORD, ServerClientWorkThread*>::iterator it;
        for (it = mapClientThread.begin();
             it != mapClientThread.end();
             ++it){
             it->second->UdateChannelErrorMode(config);
        }
    }else{
        if (FMsgHandler != NULL){
            FMsgHandler->UdateChannelErrorMode(config);
        }
    }
}
