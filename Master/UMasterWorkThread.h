//---------------------------------------------------------------------------

#ifndef UMasterWorkThreadH
#define UMasterWorkThreadH

#include <Classes.hpp>
#include <ScktComp.hpp>
#include "UMsgQueue.h"
#include "UMsg.h"
#include "UWorkThread.h"
#include "LogFileEx.h"

#define MESSAGE_BUF_LEN 100

int BuildNetMessage(
        const Msg* pmsg,
        unsigned char* netmsgbuf, unsigned int buflen);
Msg* ResloveNetMessage(
        const unsigned char* netmsgbuf, const unsigned int buflen
        );

class MasterWorkThread;
typedef void __fastcall (__closure *TRxMasterMsgEvent)(MasterWorkThread* Sender,
                                                   int msgcnt);
typedef void __fastcall (__closure *TTxMasterMsgEvent)(MasterWorkThread* Sender,
                                                   int msgcnt);

typedef enum _work_status_t{
    WORK_STATUS_WAIT,           // Wait to connect
    WORK_STATUS_DELAYCONNECT,   //
    WORK_STATUS_CONNECT,        // Wait to connected
    WORK_STATUS_LISTEN,         // Wait client to connect in server mode only.
    WORK_STATUS_WORKING,        // Wait receive and send message, go to connect while error occur.
    WORK_STATUS_STOP            // Stoping and goto wait
}work_status_t;

class MessageHandler{
public:    
    virtual void __fastcall ProcessMessage(TCustomWinSocket* client,
        TWinSocketStream *stream) = 0;
};
class MasterWorkThread : public TThread, public MessageHandler
{
private:
    MsgQueue* queue;
    bool readable;
    bool writeable;
    unsigned char receiveBuf[MESSAGE_BUF_LEN];
    unsigned char sendBuf[MESSAGE_BUF_LEN];
    int receivePos;
    int messageLen;
    int sendPos;
    int sendMessageLen;
    unsigned int lastReportTick;
    TRxMasterMsgEvent FOnRxMsg;
    TTxMasterMsgEvent FOnTxMsg;

    TOpenChannelEvent FOnServerOpen;
    TOpenChannelEvent FOnOpenChannel;
    TCloseChannelEvent FOnCloseChannel;

    AnsiString FIp;
    int FPort;
    work_status_t FStatus;

    // Master work mode
    bool FServerMode;

    // Server object and their client object
    TServerSocket* mServer;
    TCustomWinSocket* mClientPeer;
    // Client Object

    TClientSocket* mClient;


    TCriticalSection* csVariant;
    TCriticalSection* csWorkVar;
    TStringList* FThreadList;


    void dispatchMsgSafe(Msg* pmsg);

    void Init();

    // Client socket event
    void __fastcall onSocketConnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketDisconnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketRead(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketWrite(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketError(System::TObject* Sender,
        TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void __fastcall onServerListen(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall onGetThread(System::TObject* Sender,
            TServerClientWinSocket* ClientSocket,
            TServerClientThread* &SocketThread);

    void __fastcall onSendMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
    Msg * __fastcall onReceiveMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);

    TWinSocketStream *pStream;   // Socket stream in blocking mode

    
    unsigned int txMsgCnt;
    unsigned int rxMsgCnt;
    int recvLen;
    bool FAutoReconnect;

    void __fastcall Open(int port);
    void __fastcall Open(AnsiString ip, int port);
    void __fastcall Close();
public:
    __fastcall MasterWorkThread();
    __fastcall virtual ~MasterWorkThread(void);

    void __fastcall InitConnect(int port);
    void __fastcall InitConnect(AnsiString ip, int port);
    void __fastcall StartWorking();
    void __fastcall StopWorking();

    __property TRxMasterMsgEvent OnRxMsg  = { read=FOnRxMsg, write=FOnRxMsg };
    __property TTxMasterMsgEvent OnTxMsg  = { read=FOnTxMsg, write=FOnTxMsg };
    __property TCriticalSection* WorkVar  = { read=csWorkVar, write = csWorkVar };
    __property TStringList* ThreadList  = { read=FThreadList, write = FThreadList };

    __property TOpenChannelEvent OnServerOpen  = { read=FOnServerOpen, write=FOnServerOpen };
    __property TOpenChannelEvent OnOpenChannel  = { read=FOnOpenChannel, write=FOnOpenChannel };
    __property TCloseChannelEvent OnCloseChannel  = { read=FOnCloseChannel, write=FOnCloseChannel };

    //void Writeable(bool val); // socket enable to write again
    IQueue* GetQueue();  // Return Message Queue

    virtual void __fastcall Execute(void);
    __property bool AutoReconnect  = { read=FAutoReconnect, write=FAutoReconnect };

    
    virtual void __fastcall ProcessMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
    void dispatchMsg(Msg* pmsg);
    
    void LogRxMsg(Msg* pmsg);
    void LogTxMsg(Msg* pmsg);
};

class ServerClientWorkThread : public TServerClientThread
{
private:
    MasterWorkThread* FMaster;

    int receivePos;
    int messageLen;
    int sendPos;
    int sendMessageLen;

    unsigned char receiveBuf[MESSAGE_BUF_LEN];
    unsigned char sendBuf[MESSAGE_BUF_LEN];

    unsigned int txMsgCnt;
    unsigned int rxMsgCnt;
    int recvLen;

    unsigned int lastReportTick;

    IQueue* queue;
    void __fastcall Close();
    void __fastcall ProcessMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
    void __fastcall onSendMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
    Msg * __fastcall onReceiveMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
public:
    ServerClientWorkThread(
        TServerClientWinSocket* ASocket,
        MasterWorkThread* master);
protected:
    virtual void __fastcall ClientExecute(void);
};

//---------------------------------------------------------------------------
#endif
 