//---------------------------------------------------------------------------

#ifndef UMasterWorkThreadH
#define UMasterWorkThreadH

#include <Classes.hpp>
#include <ScktComp.hpp>
#include "UMsgQueue.h"
#include "UMsg.h"
#include "UWorkThread.h"
#include "LogFileEx.h"
#include "UMasterMessageHandler.h"




class MasterWorkThread;
typedef void __fastcall (__closure *TRxMasterMsgEvent)(MasterWorkThread* Sender,
                                                   int msgcnt);
typedef void __fastcall (__closure *TTxMasterMsgEvent)(MasterWorkThread* Sender,
                                                   int msgcnt);
typedef void __fastcall (__closure *TTextMessageEvent)(int source, AnsiString msg);
#define TEXT_MSG_SRC_CLIENT_INFO     1
#define TEXT_MSG_SRC_OP_ERROR        2
#define TEXT_MSG_SRC_OP_ERROR_CLIENT 3
#define TEXT_MSG_SRC_CLIENT_NOTIFY   4

class ServerClientWorkThread;
class MasterWorkThread : public TThread
{
private:
    TRxMasterMsgEvent FOnRxMsg;
    TTxMasterMsgEvent FOnTxMsg;

    TOpenChannelEvent FOnServerOpen;
    TOpenChannelEvent FOnOpenChannel;
    TCloseChannelEvent FOnCloseChannel;
    TTextMessageEvent FOnTextMessage;

    MsgQueue* queue;
    
    AnsiString FIp;
    int FPort;
    work_status_t FStatus;

    // Master work mode
    bool FServerMode;

    // Server object and their client object
    TServerSocket* mServer;
    TCustomWinSocket* mClientPeer;

    MasterMessageHandler* FMsgHandler;

    TClientSocket* mClient;

    ServerClientWorkThread* currSocketThread; // Client Work Thread for server mode

    TCriticalSection* csVariant;
    TCriticalSection* csWorkVar;
    TStringList* FThreadList; //Set by main program


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
    void __fastcall onServerAccept(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall onGetThread(System::TObject* Sender,
            TServerClientWinSocket* ClientSocket,
            TServerClientThread* &SocketThread);

    int FSource;
    AnsiString FTextMessage;
    void __fastcall UpdateTextMessageUI(void);
    
    void CloseClientConnection();


    TWinSocketStream *pStream;   // Socket stream in blocking mode

    
    unsigned int txMsgCnt;
    unsigned int rxMsgCnt;
    int recvLen;
    bool FAutoReconnect;

    void __fastcall Open(int port);
    void __fastcall Open(AnsiString ip, int port);
    void __fastcall Close();

    void LogMsg(AnsiString msg);
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
    __property TTextMessageEvent OnTextMessage  = { read=FOnTextMessage, write=FOnTextMessage };



    //void Writeable(bool val); // socket enable to write again
    IQueue* GetQueue();  // Return Message Queue

    virtual void __fastcall Execute(void);
    __property bool AutoReconnect  = { read=FAutoReconnect, write=FAutoReconnect };


    void dispatchMsg(Msg* pmsg);
    
    void LogRxMsg(Msg* pmsg);
    void LogTxMsg(Msg* pmsg);
};



//---------------------------------------------------------------------------
#endif
 