//---------------------------------------------------------------------------

#ifndef UMasterWorkThreadH
#define UMasterWorkThreadH

#include <Classes.hpp>
#include <ScktComp.hpp>
#include <map>
#include "UMsgQueue.h"
#include "UMsg.h"
#include "UWorkThread.h"
#include "LogFileEx.h"
#include "UMasterMessageHandler.h"


using namespace std;

class MasterWorkThread;
typedef void __fastcall (__closure *TMasterOpenEvent)(int ch,
                                                   bool opened);
typedef void __fastcall (__closure *TMasterCloseEvent)(int ch,
                                                   bool closed);
typedef void __fastcall (__closure *TRxMasterMsgEvent)(int ch,
                                                   int msgcnt);
typedef void __fastcall (__closure *TTxMasterMsgEvent)(int ch,
                                                   int msgcnt);
typedef void __fastcall (__closure *TTextMessageEvent)(int ch, int source, AnsiString msg);
#define TEXT_MSG_SRC_CLIENT_INFO     1
#define TEXT_MSG_SRC_OP_ERROR        2
#define TEXT_MSG_SRC_OP_ERROR_CLIENT 3
#define TEXT_MSG_SRC_CLIENT_NOTIFY   4
#define TEXT_MSG_SRC_CLIENT_COUNT    5

class ServerClientWorkThread;
class MasterWorkThread : public TThread
{
private:
    TRxMasterMsgEvent FOnRxMsg;
    TTxMasterMsgEvent FOnTxMsg;

    TMasterOpenEvent FOnServerOpen;
    TMasterOpenEvent FOnOpenChannel;
    TMasterCloseEvent FOnCloseChannel;
    TTextMessageEvent FOnTextMessage;

    Controller* FController;
    //Channel* FChannel;
    //MsgQueue* queue; // Only for client mode
    //Channel* FChannel;
    
    AnsiString FIp;
    int FPort;
    int FCh; //Channel index
    int FPriority;
    work_status_t FStatus;

    // Master work mode
    bool FServerMode;
    bool FStartWorkingMode;

    // Server object and their client object
    TServerSocket* mServer;
    TCustomWinSocket* mClientPeer;

    MasterMessageHandler* FMsgHandler;  //Only for client mode


    TClientSocket* mClient;

    ServerClientWorkThread* currSocketThread; // Client Work Thread for server mode
    map<DWORD, ServerClientWorkThread*> mapClientThread;

    TCriticalSection* csVariant;
    TCriticalSection* csWorkVar;
    TStringList* FThreadList; //Set by main program


    //void dispatchMsgSafe(ServerClientWorkThread* client, Msg* pmsg);
    //bool DispatchToInner(AnsiString& dest, RawMsg* rawmsg);

    void Init();
    void BuildTagListMsg(Msg* msg);

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
    void __fastcall UpdateConnectStatus();
    
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
    __fastcall MasterWorkThread(int ch, Controller* controller, int priority = 0);
    __fastcall virtual ~MasterWorkThread(void);

    void __fastcall InitConnect(int port);
    void __fastcall InitConnect(AnsiString ip, int port);
    void __fastcall StartWorking();
    void __fastcall StopWorking();
    bool __fastcall isTerminated();
    

    __property int Channel  = { read=FCh, write=FCh };
    __property TRxMasterMsgEvent OnRxMsg  = { read=FOnRxMsg, write=FOnRxMsg };
    __property TTxMasterMsgEvent OnTxMsg  = { read=FOnTxMsg, write=FOnTxMsg };
    __property TCriticalSection* WorkVar  = { read=csWorkVar, write = csWorkVar };
    __property TStringList* ThreadList  = { read=FThreadList, write = FThreadList };
    __property int Priority  = { read=FPriority, write = FPriority };

    __property TMasterOpenEvent OnServerOpen  = { read=FOnServerOpen, write=FOnServerOpen };
    __property TMasterOpenEvent OnOpenChannel  = { read=FOnOpenChannel, write=FOnOpenChannel };
    __property TMasterCloseEvent OnCloseChannel  = { read=FOnCloseChannel, write=FOnCloseChannel };
    __property TTextMessageEvent OnTextMessage  = { read=FOnTextMessage, write=FOnTextMessage };


    //Get channel priority
    int GetChannelPriority();
    //Update channel error mode
    void UdateChannelErrorMode(const master_config_t* config);
    
    virtual void __fastcall Execute(void);
    __property bool AutoReconnect  = { read=FAutoReconnect, write=FAutoReconnect };


    virtual void Push(Msg* pmsg);
    //virtual void dispatchMsg(ServerClientWorkThread* client, Msg* pmsg);
    //virtual void dispatchMsg(Msg* pmsg);
    
    void LogRxMsg(Msg* pmsg);
    void LogTxMsg(Msg* pmsg);
    void LogMsgStream(AnsiString& stream);
    void LogMsg(Msg* pmsg, char* tag);
};



//---------------------------------------------------------------------------
#endif
 