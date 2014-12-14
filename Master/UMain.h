//---------------------------------------------------------------------------

#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Grids.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <ScktComp.hpp>
#include <map>
#include <queue>

#include "UWorkThread.h"
#include "UMasterWorkThread.h"
#include "LogFileEx.h"

using namespace std;

//-- message defines ---------------------------------------------------------
#define WM_USER_OFFSET              0x100
#define WM_UPDATE_OPEN_STATUS       (WM_USER + WM_USER_OFFSET + 1)
#define WM_UPDATE_CLOSE_STATUS      (WM_USER + WM_USER_OFFSET + 2)
#define WM_UPDATE_RX_MSG_CNT        (WM_USER + WM_USER_OFFSET + 3)
#define WM_UPDATE_TX_MSG_CNT        (WM_USER + WM_USER_OFFSET + 4)
#define WM_UPDATE_ERR_MSG_CNT       (WM_USER + WM_USER_OFFSET + 5)
#define WM_UPDATE_MASTER_RX_BYTES   (WM_USER + WM_USER_OFFSET + 6)
#define WM_UPDATE_MASTER_TX_BYTES   (WM_USER + WM_USER_OFFSET + 7)
#define WM_UPDATE_SERVER_OPEN       (WM_USER + WM_USER_OFFSET + 8)
#define WM_UPDATE_MASTER_OPEN       (WM_USER + WM_USER_OFFSET + 9)
#define WM_UPDATE_MASTER_CLOSE      (WM_USER + WM_USER_OFFSET + 10)
#define WM_UPDATE_MASTER_SERVER_OPEN (WM_USER + WM_USER_OFFSET + 12)

//---------------------------------------------------------------------------
#define BUTTON_STATUS_START 0x00000000
#define BUTTON_STATUS_STOP  0x80000000
//---------------------------------------------------------------------------
typedef struct WorkItem{
    TBitBtn* button;
    WorkThread* thread;
}WorkItem;

typedef struct buffer_t{
    unsigned char *pbuff;
             int  len;
}buffer_t;

extern LogFileEx logger;
//---------------------------------------------------------------------------
class TFMain : public TForm
{
__published:	// IDE-managed Components
    TGroupBox *GroupBox1;
    TStringGrid *gridDevices;
    TSplitter *Splitter1;
    TGroupBox *Master;
    TRadioButton *rbMasterClientMode;
    TRadioButton *rbMasterServerMode;
    TLabel *lblPeerIP;
    TEdit *txtPeerIP;
    TLabel *lblPeerPort;
    TEdit *txtPeerPort;
    TLabel *lblMasterPort;
    TEdit *txtMasterPort;
    TLabel *lblPeerClientIP;
    TEdit *txtPeerClientIP;
    TLabel *lblConnectStatus;
    TLabel *lblListenStatus;
    TLabel *Label7;
    TLabel *Label8;
    TLabel *Label9;
    TEdit *txtRxBytes;
    TEdit *txtTxBytes;
    TLabel *Label10;
    TLabel *Label11;
    TLabel *lblRxRate;
    TLabel *lblTxRate;
    TMainMenu *mnuMain;
    TMenuItem *File1;
    TMenuItem *Help1;
    TMenuItem *Export1;
    TMenuItem *OpenLogFile1;
    TMenuItem *N1;
    TMenuItem *Exit1;
    TMenuItem *About1;
    TMenuItem *TCPclientconfigure1;
    TMenuItem *Saveconfigure1;
    TBitBtn *btnOpen;
    TBitBtn *btnConnect;
    TCheckBox *chkAutoReconn;
    TTimer *tmrReconn;
    TButton *btnClear;
    void __fastcall Exit1Click(TObject *Sender);
    void __fastcall rbMasterClientModeClick(TObject *Sender);
    void __fastcall rbMasterServerModeClick(TObject *Sender);
    void __fastcall Saveconfigure1Click(TObject *Sender);
    void __fastcall gridDevicesTopLeftChanged(TObject *Sender);
    void __fastcall btnOpenClick(TObject *Sender);
    void __fastcall ServerMasterListen(TObject *Sender,
          TCustomWinSocket *Socket);
    void __fastcall ServerMasterAccept(TObject *Sender,
          TCustomWinSocket *Socket);
    void __fastcall btnConnectClick(TObject *Sender);
    void __fastcall tmrReconnTimer(TObject *Sender);
    void __fastcall btnClearClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall chkAutoReconnClick(TObject *Sender);
private:	// User declarations
    map<int, WorkItem> mWorkItems;
    TCriticalSection* csWorkVar;
    TStringList* lstThreadObj;
    
    WorkThread* __fastcall CreateWorkThread(int rowidx);

    void UpdateOperationUI();
    // configure
    void ReloadConfigure();
    void SaveConfigure();
    // all threads control
    void StartAllThread();
    void StopAllThread();
    void TerminateAllThread();
    // active and passive all threads
    void SetAllThreadActive(bool active);
    // Edit item
    bool EditRow(int rowidx);
    
    void __fastcall OperationButtonClick(TObject *Sender);

    // Trafic statisic
    unsigned int mRxBytes;
    unsigned int mTxBytes;
    queue<buffer_t> qSend;
    int qFrontSentBytes;

    // Master work thread
    MasterWorkThread* masterThread;

    //Calc Rx and Tx statics
    unsigned int mRxStartTick;
    unsigned int mTxStartTick;


    // Callback function from work thread
    void __fastcall onServerOpenChannel(WorkThread* Sender, bool opened);
    void __fastcall onOpenChannel(WorkThread* Sender, bool opened);
    void __fastcall onCloseChannel(WorkThread* Sender, bool closed);
    void __fastcall onRxMsg(WorkThread* Sender, int msgcnt);
    void __fastcall onTxMsg(WorkThread* Sender, int msgcnt);
    //void __fastcall onErrMsg(WorkThread* Sender, int msgcnt);
    void __fastcall onMasterRxMsg(MasterWorkThread* Sender, int msgcnt);
    void __fastcall onMasterTxMsg(MasterWorkThread* Sender, int msgcnt);
    void __fastcall onMasterOpenChannel(WorkThread* Sender, bool opened);
    void __fastcall onMasterCloseChannel(WorkThread* Sender, bool closed);
    void __fastcall onMasterServerOpen(WorkThread* Sender, bool closed);

    void __fastcall UpdateOpenStatus(TMessage* Msg);
    void __fastcall UpdateCloseStatus(TMessage* Msg);
    void __fastcall UpdateRxMsgCnt(TMessage* Msg);
    void __fastcall UpdateTxMsgCnt(TMessage* Msg);
    void __fastcall UpdateErrMsgCnt(TMessage* Msg);
    void __fastcall UpdateServerOpen(TMessage* Msg);

    void __fastcall UpdateMasterTxMsgCnt(TMessage* Msg);
    void __fastcall UpdateMasterRxMsgCnt(TMessage* Msg);
    void __fastcall UpdateMasterOpenStatus(TMessage* Msg);
    void __fastcall UpdateMasterCloseStatus(TMessage* Msg);
    void __fastcall UpdateMasterServerOpen(TMessage* Msg);
public:		// User declarations
    __fastcall TFMain(TComponent* Owner);\

    // Message process routine
    virtual void __fastcall Dispatch(void *Message);
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------
#endif
