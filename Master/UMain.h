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
#include <ComCtrls.hpp>
#include <map>
#include <queue>
#include <list>

#include "UWorkThread.h"
#include "UMasterWorkThread.h"
#include "LogFileEx.h"
#include "UComm.h"
#include "UController.h"

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
    TLabel *lblRxBytes;
    TLabel *RxBytes;
    TEdit *txtRxBytesCH1;
    TEdit *txtTxBytesCH1;
    TLabel *lblRxRate1;
    TLabel *lblTxRate1;
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
    TTimer *tmrOutputPri;
    TButton *btnClear;
    TLabel *lblConnCntCH1;
    TEdit *txtRxBytesCH2;
    TEdit *txtTxBytesCH2;
    TLabel *lblTxRate2;
    TLabel *lblRxRate2;
    TEdit *txtRxBytesCH3;
    TEdit *txtTxBytesCH3;
    TLabel *lblTxRate3;
    TLabel *lblRxRate3;
    TShape *Shape1;
    TLabel *lblCh1;
    TLabel *lblCH2;
    TLabel *lblCH3;
    TLabel *lblConnCntCH2;
    TLabel *lblConnCntCH3;
    TGroupBox *gboxServerMode;
    TLabel *lblChanneControl;
    TComboBox *cboErrorCH1;
    TLabel *lblMasterPort;
    TEdit *txtMasterPort;
    TComboBox *cboErrorCH2;
    TComboBox *cboErrorCH3;
    TBitBtn *btnOpen;
    TLabel *lblListenStatus;
    TGroupBox *GroupBox2;
    TLabel *lblPeerIP;
    TEdit *txtPeerIP;
    TLabel *lblPeerPort;
    TEdit *txtPeerPort;
    TBitBtn *btnConnect;
    TLabel *lblConnectStatus;
    TCheckBox *chkAutoReconn;
    TShape *ShapeCH11;
    TShape *ShapeCH12;
    TShape *ShapeCH13;
    TShape *ShapeCH21;
    TShape *ShapeCH22;
    TShape *ShapeCH23;
    TShape *ShapeCH31;
    TShape *ShapeCH32;
    TShape *ShapeCH33;
    TComboBox *cboErrorValCH1;
    TComboBox *cboErrorValCH2;
    TComboBox *cboErrorValCH3;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TMemo *txtResult;
    TButton *btnResult;
    TTimer *tmrWriteResult;
    TRadioButton *rbRetransSameCh;
    TRadioButton *RadioButton1;
    TEdit *txtRetransCount;
    TCheckBox *chkLimitRetrans;
    TLabel *lblRetransCount;
    void __fastcall Exit1Click(TObject *Sender);
    void __fastcall rbMasterClientModeClick(TObject *Sender);
    void __fastcall rbMasterServerModeClick(TObject *Sender);
    void __fastcall Saveconfigure1Click(TObject *Sender);
    void __fastcall gridDevicesTopLeftChanged(TObject *Sender);
    void __fastcall btnOpenClick(TObject *Sender);
    void __fastcall btnConnectClick(TObject *Sender);
    void __fastcall btnClearClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall chkAutoReconnClick(TObject *Sender);
    void __fastcall tmrOutputPriTimer(TObject *Sender);
    void __fastcall cboErrorCH1Change(TObject *Sender);
    void __fastcall cboErrorCH2Change(TObject *Sender);
    void __fastcall cboErrorCH3Change(TObject *Sender);
    void __fastcall cboErrorValCH1Change(TObject *Sender);
    void __fastcall cboErrorValCH2Change(TObject *Sender);
    void __fastcall cboErrorValCH3Change(TObject *Sender);
    void __fastcall btnResultClick(TObject *Sender);
    void __fastcall txtResultDblClick(TObject *Sender);
    void __fastcall tmrWriteResultTimer(TObject *Sender);
    void __fastcall chkLimitRetransClick(TObject *Sender);
    void __fastcall rbRetransSameChClick(TObject *Sender);
    void __fastcall RadioButton1Click(TObject *Sender);
    void __fastcall txtRetransCountChange(TObject *Sender);
private:	// User declarations
    String FName;
    map<int, WorkItem> mWorkItems;
    TCriticalSection* csWorkVar;
    TStringList* lstThreadObj;
    
    WorkThread* __fastcall CreateWorkThread(int rowidx);
    list<device_config_t*> lstDeviceConfig; // Device configure list
    master_config_t masterConfig[3];

    void __fastcall CreateUI();
    void __fastcall UpdateUI();
    void __fastcall ReInitAllDeviceConfigure();
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
    unsigned int mRxBytes[3];
    unsigned int mTxBytes[3];
    queue<buffer_t> qSend;
    int qFrontSentBytes;

    // Master work thread
    MasterWorkThread* masterThread[3];

    //Controller
    Controller mController;

    //Calc Rx and Tx statics
    unsigned int mRxStartTick;
    unsigned int mTxStartTick;

    // Channel priority
    int iChPri[3];


    // Callback function from work thread
    void __fastcall onServerOpenChannel(WorkThread* Sender, bool opened);
    void __fastcall onOpenChannel(WorkThread* Sender, bool opened);
    void __fastcall onCloseChannel(WorkThread* Sender, bool closed);
    void __fastcall onRxMsg(WorkThread* Sender, int msgcnt);
    void __fastcall onTxMsg(WorkThread* Sender, int msgcnt);
    //void __fastcall onErrMsg(WorkThread* Sender, int msgcnt);
    void __fastcall onMasterRxMsg(int ch, int msgcnt);
    void __fastcall onMasterTxMsg(int ch, int msgcnt);
    void __fastcall onMasterOpenChannel(int ch, bool opened);
    void __fastcall onMasterCloseChannel(int ch, bool closed);
    void __fastcall onMasterServerOpen(int ch, bool closed);
    void __fastcall onTextMessage(int ch, int source, AnsiString msg);


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

    int GetMasterCHTxBytes(int ch);
    void SetMasterCHTxBytes(int ch, int val);
    int GetMasterCHRxBytes(int ch);
    void SetMasterCHRxBytes(int ch, int val);
    void SetMasterCHColor(int ch, TColor color);
    void SetMasterCHConnections(int ch, AnsiString val);
    void SetRxRate(int ch, float rate);
    void SetTxRate(int ch, float rate);

    short GetMasterCHPort(int ch);
    short GetPeerCHPort(int ch);

    void UpdateChannelPriUI();
    void UpdateChannelErrorMode(int ch);
    int GetMaxPriChannel();

    void __fastcall SaveSimulateResult();
public:		// User declarations
    __fastcall TFMain(TComponent* Owner);\

    // Message process routine
    virtual void __fastcall Dispatch(void *Message);
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------
#endif
