//---------------------------------------------------------------------------


#pragma hdrstop
#include <QDialogs.hpp>
#include <stdio.h>
#include "UMasterWorkThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define NETMESSAGE_MINLEN (8+sizeof(RawMsg))
#define MAX_ALIAS_LEN 20   

extern LogFileEx logger;

int BuildNetMessage(
        const Msg* pmsg,
        unsigned char* netmsgbuf, unsigned int buflen)
{
    /** net message length: 1 bytes + from alias length(include end char)
        + to alias bytes(include end char) + message
    **/
    if (pmsg == NULL) return -1;
    unsigned char totallen = 1 + 1 + strlen(pmsg->from) + 1 + strlen(pmsg->to)
        + sizeof(RawMsg);
    if (buflen < (unsigned int)totallen){
        return -1;
    }else{
        unsigned char* pwrite = netmsgbuf;
        unsigned char alen;
        memcpy(pwrite, &totallen, 1); pwrite++;
        alen = strlen(pmsg->from)+1;
        memcpy(pwrite, pmsg->from, alen); pwrite += alen;
        alen = strlen(pmsg->to)+1;
        memcpy(pwrite, pmsg->to, alen); pwrite += alen;
        memcpy(pwrite, pmsg->rawmsg.message, sizeof(RawMsg));

        return (int)totallen & 0xFF;
    }
}

static Msg* ResloveNetMessage(
        const unsigned char* netmsgbuf, const unsigned int buflen
        )
{
    static char alias[MAX_ALIAS_LEN];
    if (buflen < NETMESSAGE_MINLEN){
        return NULL; // No more data to reslove
    }else{
        unsigned char totallen;

        const unsigned char* pread = netmsgbuf;
        memcpy(&totallen, pread, 1); pread ++;
        if ((unsigned int)totallen > buflen){
            return NULL; // No more data to reslove
        }else{
            char from[ALIAS_LEN], to[ALIAS_LEN];
            RawMsg msg;
            
            memset(alias, 0, sizeof(alias));
            strcpy(alias, pread);
            pread += strlen(alias)+1;
            strncpy(from, alias, ALIAS_LEN);
            
            memset(alias, 0, sizeof(alias));
            strcpy(alias, pread);
            pread += strlen(alias)+1;
            strncpy(to, alias, ALIAS_LEN);

            memcpy(msg.message, pread, sizeof(msg.message));

            return new Msg(from, to, msg);
        }
    }
}

void MasterWorkThread::Init()
{
    csVariant = new TCriticalSection();
    readable = false;
    writeable = true; // default to write
    FAutoReconnect = true;
    mClientPeer = NULL;
    memset(receiveBuf, 0, sizeof(receiveBuf));
    receivePos = 0;
    messageLen = 0; // Start to receive a new message
    lastReportTick = 0;

    txMsgCnt = 0;
    rxMsgCnt = 0;
    recvLen = 0;

    FStatus = WORK_STATUS_WAIT;
    
    queue = new MsgQueue();

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
    mServer->ServerType = stThreadBlocking;
    mServer->OnListen = onServerListen;
    try{
        mServer->Active = true;



        readable = false;
        writeable = true;

        FStatus = WORK_STATUS_LISTEN;
    }catch(...){
        logger.Log(IntToStr(::GetCurrentThreadId()) + "Open failure.");
        Close();
    }
}

void __fastcall MasterWorkThread::Open(AnsiString ip, int port) 
{
    FServerMode = false;
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
    }catch(...){
        logger.Log(IntToStr(::GetCurrentThreadId()) + "Open failure.");
        Close();
    }
    readable = false;
    writeable = true; 
}

void __fastcall MasterWorkThread::Close()
{
    if (FServerMode){
        if (mServer != NULL && mServer->Active){
            try{
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
    messageLen = 0;
    receivePos = 0;
    sendMessageLen = 0;
    sendPos = 0;
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
}

IQueue* MasterWorkThread::GetQueue()
{
    return queue;
}
//---------------------------------------------------------------------------
void __fastcall MasterWorkThread::onSendMessage(TCustomWinSocket* client,
    TWinSocketStream *stream)
{
    
    if (client != NULL ){
        if (sendMessageLen == 0 && !queue->Empty()){
            Msg *pmsg = queue->Pop();
            // build message
            sendMessageLen = BuildNetMessage(pmsg, sendBuf, MESSAGE_BUF_LEN);
            // logger
            LogTxMsg(pmsg);
            delete pmsg;
        }
        try{
            int writeLen = stream->Write(sendBuf + sendPos,
                    sendMessageLen - sendPos);
            if(writeLen < 0){
                logger.Log("Write data error!");
            }
            sendPos += writeLen;
            txMsgCnt += writeLen;
            if (sendPos >= sendMessageLen){
                // a message has sent
                sendMessageLen = 0;
                sendPos = 0;
            }
        }catch(...){
            logger.Log("Wait for data to write failure.");
            Close();
        }
    }
}
Msg * __fastcall MasterWorkThread::onReceiveMessage(TCustomWinSocket* client,
    TWinSocketStream *stream)
{
    if(client != NULL && stream != NULL){
        if (stream->WaitForData(1)){
            if (messageLen <= 0){
                try{
                    recvLen = stream->Read(receiveBuf, 1);
                }catch(...){
                    logger.Log("Master read error.");
                    Close();
                }
                if (recvLen > 0){
                    receivePos += recvLen;
                    rxMsgCnt += recvLen;

                    messageLen = receiveBuf[0] & 0xFF;
                }else{
                    // Connect disconnect
                    Close();
                }
            }else{
                try{
                    recvLen = stream->Read(receiveBuf + receivePos,
                                        messageLen - receivePos);
                }catch(...){
                    logger.Log("Master read error.");
                    Close();
                }
                if (recvLen > 0){
                    receivePos += recvLen;
                    rxMsgCnt += recvLen;

                    if (receivePos == messageLen){
                        // get a message, neet to post to queue
                        Msg *pmsg = ResloveNetMessage(receiveBuf, messageLen);

                        messageLen = 0;
                        receivePos = 0;

                        return pmsg;
                    }
                }else{
                    logger.Log("No bytes has received.");
                }
            }//~messageLen > 0
        }
    }
    return NULL;
}

void __fastcall MasterWorkThread::ProcessMessage(TCustomWinSocket* client,
    TWinSocketStream *stream)
{
    try{
        Msg* pmsg = onReceiveMessage(client, stream);
        if (pmsg != NULL){
            // logger
            LogRxMsg(pmsg);
            // dispathch to work thread
            dispatchMsg(pmsg);
            delete pmsg;
        }
        // write message to peer
        if (client != NULL ){
            onSendMessage(client, stream);//
        }

        // Every second report receive and send message count
        if (::GetTickCount() - lastReportTick >= 100){
            if (OnTxMsg != NULL && txMsgCnt > 0){
                OnTxMsg(this, txMsgCnt);
                txMsgCnt = 0;
            }
            if (OnRxMsg != NULL && rxMsgCnt > 0){
                OnRxMsg(this, rxMsgCnt);
                rxMsgCnt = 0;
            }
            lastReportTick = ::GetTickCount();
        }
        //Sleep(10);
    }catch(...){
        logger.Log("master exception, set null.");
        // write error will disconnect socket
        //mClientPeer = NULL;
    }
}
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
                if (mServer->Socket->ActiveConnections > 0){
                    //mClientPeer = mServer->Socket->Connections[0];
                    //pStream = new TWinSocketStream(mClientPeer, 3000);
                    //FStatus = WORK_STATUS_WORKING;
                }
                /*if (mServer->Socket->ActiveConnections > 1){
                    try{
                        mServer->Socket->Lock();
                        for (int i = 0; i < mServer->Socket->ActiveConnections; i++){
                            if (mServer->Socket->Connections[i] != mClientPeer){
                                // find a client diffient from current
                                try{
                                mClientPeer->Close();
                                }catch(...){}
                                mClientPeer = mServer->Socket->Connections[i];
                            }else{
                                try{
                                mClientPeer->Close();
                                }catch(...){}
                                mClientPeer = NULL;
                            }
                        }
                    }__finally{
                        mServer->Socket->Unlock();
                    }
                }*/
            }
            Sleep(10);
            break;
        case WORK_STATUS_WORKING:
            if (FServerMode){
                Sleep(10);
            }else{
                ProcessMessage(mClientPeer, pStream);
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
    }
}

void MasterWorkThread::LogRxMsg(Msg* pmsg)
{
    static char buffer[100];
    snprintf(buffer, 100, "Rx:[%s->%s] L:%d", pmsg->from
        , pmsg->to, sizeof(pmsg->rawmsg.message));
    logger.Log(buffer);
}

void MasterWorkThread::LogTxMsg(Msg* pmsg)
{
    static char buffer[100];
    snprintf(buffer, 100, "Tx:[%s->%s] L:%d", pmsg->from
        , pmsg->to, sizeof(pmsg->rawmsg.message));
    logger.Log(buffer);
}

//---------------------------------------------------------------------------
void __fastcall MasterWorkThread::onSocketConnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    logger.Log(IntToStr(::GetCurrentThreadId()) + ", Master connected:" + Socket->RemoteAddress + "Handle:" + IntToStr(Socket->Handle));
    // Close pervious connection
    if (mClientPeer != NULL){
        Close();
    }

    if (FOnOpenChannel != NULL){
        FOnOpenChannel(NULL, true);
    }
    if (FServerMode){
        mClientPeer = Socket;
    }else{
        mClientPeer = mClient->Socket;
        pStream = new TWinSocketStream(mClientPeer, 3000);
    }
    FStatus = WORK_STATUS_WORKING;
}
void __fastcall MasterWorkThread::onSocketDisconnect(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    logger.Log("Master disconnected:" + IntToStr(Socket->Handle));
    if (FServerMode && mServer != NULL){
        if (mClientPeer == Socket){
            mClientPeer = NULL;
        }
    }else{
        mClientPeer = NULL;
    }
    if (FOnCloseChannel != NULL){
        FOnCloseChannel(NULL, true);
    }
}
void __fastcall MasterWorkThread::onSocketRead(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    logger.Log("Master onSocketRead:" + IntToStr(::GetCurrentThreadId()));
    readable = true;
}
void __fastcall MasterWorkThread::onSocketWrite(System::TObject* Sender,
    TCustomWinSocket* Socket)
{
    writeable = true;
}

void __fastcall MasterWorkThread::onSocketError(System::TObject* Sender,
    TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
    logger.Log("Socket error:" + IntToStr(Socket->Handle));
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
    ErrorCode = 0;
}
void __fastcall MasterWorkThread::onServerListen(TObject *Sender, TCustomWinSocket *Socket)
{
    if (OnServerOpen != NULL){
        OnServerOpen(NULL, true);

        /*mServer->Socket->OnClientConnect = onSocketConnect;
        mServer->Socket->OnClientDisconnect = onSocketDisconnect;
        mServer->Socket->OnClientRead = onSocketRead;
        mServer->Socket->OnClientWrite = onSocketWrite;
        mServer->Socket->OnClientError = onSocketError;*/
        mServer->Socket->OnGetThread = onGetThread;
        //mServer->Socket->OnThreadEnd = onThreadEnd;

        //Ready to accept
        //mServer->Socket->Accept(mServer->Socket->SocketHandle);
        //int errcode = WSAGetLastError();
        //logger.Log("WSAGetLastError:" + IntToStr(errcode));
    }
}
//---------------------------------------------------------------------------
void __fastcall MasterWorkThread::onGetThread(System::TObject* Sender,
            TServerClientWinSocket* ClientSocket,
            TServerClientThread* &SocketThread)
{
    //logger.Log("Client.....");
    try{
        mServer->Socket->Lock();
        if (mServer->Socket->ActiveConnections > 1){
            ServerClientWorkThread* thread = (ServerClientWorkThread*)mServer->Socket->GetClientThread((TServerClientWinSocket*)mServer->Socket->Connections[0]);
            thread->Terminate();
        }
    }__finally{
        mServer->Socket->Unlock();
    }
    SocketThread = new ServerClientWorkThread(ClientSocket, this);
}
//---------------------------------------------------------------------------



//ServerClientWorkThread
ServerClientWorkThread::ServerClientWorkThread(TServerClientWinSocket* ASocket
    , MasterWorkThread* master) : TServerClientThread(false, ASocket)
{
    this->FMaster = master;
    queue = FMaster->GetQueue();
    
    logger.Log(IntToStr(::GetCurrentThreadId()) + ", Master connected:"
        + ASocket->RemoteAddress + "Handle:" + IntToStr(ASocket->Handle));
}

void __fastcall ServerClientWorkThread::ClientExecute(void)
{
    try{
        TWinSocketStream *pStream = new TWinSocketStream(ClientSocket, 3000);

        while(!this->Terminated && ClientSocket->Connected){
            ProcessMessage(ClientSocket, pStream);    
        }
        //logger.Log(IntToStr(::GetCurrentThreadId()) +
        //    ", ServerClientWorkThread::ClientExecute exit.");
    }catch(...){
        ClientSocket->Close();
        DoTerminate();
    }
}
void __fastcall ServerClientWorkThread::Close()
{
    receivePos = 0;
    messageLen = 0;
    sendPos = 0;
    sendMessageLen = 0;

    lastReportTick = 0;

    this->Terminate();
}
//---------------------------------------------------------------------------
void __fastcall ServerClientWorkThread::onSendMessage(TCustomWinSocket* client,
    TWinSocketStream *stream)
{
    
    if (client != NULL ){
        if (sendMessageLen == 0 && !queue->Empty()){
            Msg *pmsg = queue->Pop();
            // build message
            sendMessageLen = BuildNetMessage(pmsg, sendBuf, MESSAGE_BUF_LEN);
            // logger
            FMaster->LogTxMsg(pmsg);
            delete pmsg;
        }
        try{
            int writeLen = stream->Write(sendBuf + sendPos,
                    sendMessageLen - sendPos);
            if(writeLen < 0){
                logger.Log("Write data error!");
            }
            sendPos += writeLen;
            txMsgCnt += writeLen;
            if (sendPos >= sendMessageLen){
                // a message has sent
                sendMessageLen = 0;
                sendPos = 0;
            }
        }catch(...){
            logger.Log("Wait for data to write failure.");
            Close();
        }
    }
}
Msg * __fastcall ServerClientWorkThread::onReceiveMessage(TCustomWinSocket* client,
    TWinSocketStream *stream)
{
    if(client != NULL && stream != NULL){
        if (stream->WaitForData(1)){
            if (messageLen <= 0){
                try{
                    recvLen = stream->Read(receiveBuf, 1);
                }catch(...){
                    logger.Log("Master read error.");
                    Close();
                }
                if (recvLen > 0){
                    receivePos += recvLen;
                    rxMsgCnt += recvLen;

                    messageLen = receiveBuf[0] & 0xFF;
                }else{
                    // Connect disconnect
                    Close();
                }
            }else{
                try{
                    recvLen = stream->Read(receiveBuf + receivePos,
                                        messageLen - receivePos);
                }catch(...){
                    logger.Log("Master read error.");
                    Close();
                }
                if (recvLen > 0){
                    receivePos += recvLen;
                    rxMsgCnt += recvLen;

                    if (receivePos == messageLen){
                        // get a message, neet to post to queue
                        Msg *pmsg = ResloveNetMessage(receiveBuf, messageLen);

                        messageLen = 0;
                        receivePos = 0;

                        return pmsg;
                    }
                }else{
                    logger.Log("No bytes has received.");
                }
            }//~messageLen > 0
        }
    }
    return NULL;
}

void __fastcall ServerClientWorkThread::ProcessMessage(TCustomWinSocket* client,
    TWinSocketStream *stream)
{
    try{
        Msg* pmsg = onReceiveMessage(client, stream);
        if (pmsg != NULL){
            // logger
            FMaster->LogRxMsg(pmsg);
            // dispathch to work thread
            FMaster->dispatchMsg(pmsg);
            delete pmsg;
        }
        // write message to peer
        if (client != NULL ){
            onSendMessage(client, stream);//
        }

        // Every second report receive and send message count
        if (::GetTickCount() - lastReportTick >= 100){
            if (FMaster->OnTxMsg != NULL && txMsgCnt > 0){
                FMaster->OnTxMsg(FMaster, txMsgCnt);
                txMsgCnt = 0;
            }
            if (FMaster->OnRxMsg != NULL && rxMsgCnt > 0){
                FMaster->OnRxMsg(FMaster, rxMsgCnt);
                rxMsgCnt = 0;
            }
            lastReportTick = ::GetTickCount();
        }
        //Sleep(10);
    }catch(...){
        logger.Log("master exception, set null.");
        // write error will disconnect socket
        //mClientPeer = NULL;
    }
}
