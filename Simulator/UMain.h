//---------------------------------------------------------------------------

#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>
#include <Menus.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <map>

#include "USerialWorkThread.h"
#include "UClientWorkThread.h"
using namespace std;
//-- message defines ---------------------------------------------------------
#define WM_USER_OFFSET           0x100
#define WM_UPDATE_OPEN_STATUS   (WM_USER + WM_USER_OFFSET + 1)
#define WM_UPDATE_CLOSE_STATUS  (WM_USER + WM_USER_OFFSET + 2)
#define WM_UPDATE_RX_MSG_CNT    (WM_USER + WM_USER_OFFSET + 3)
#define WM_UPDATE_TX_MSG_CNT    (WM_USER + WM_USER_OFFSET + 4)
#define WM_UPDATE_ERR_MSG_CNT   (WM_USER + WM_USER_OFFSET + 5)

//---------------------------------------------------------------------------
#define BUTTON_STATUS_START 0x00000000
#define BUTTON_STATUS_STOP  0x80000000
//---------------------------------------------------------------------------
typedef struct WorkItem{
    TBitBtn* button;
    WorkThread* thread;
}WorkItem;
//---------------------------------------------------------------------------
class TFMain : public TForm
{
__published:	// IDE-managed Components
    TStringGrid *gridDevices;
    TMainMenu *MainMenu1;
    TMenuItem *File1;
    TMenuItem *Help1;
    TMenuItem *Openalldevices1;
    TMenuItem *Closealldevices1;
    TMenuItem *N1;
    TMenuItem *Savecurrentconfigure1;
    TMenuItem *mnuReloda;
    TMenuItem *N2;
    TMenuItem *Exit1;
    TMenuItem *About1;
    TBitBtn *btnBatchAdd;
    TBitBtn *btnAddItem;
    TBitBtn *DeleteItem;
    TBitBtn *btnSave;
    TBitBtn *btnStartAll;
    TBitBtn *btnStopAll;
    TCheckBox *chkActiveMode;
    TMenuItem *mnuActivePassive;
    TPanel *palPrompt;
    TMenuItem *N3;
    TMenuItem *AddItem1;
    TMenuItem *BatchAdd1;
    TMenuItem *DeleteItem1;
    TCheckBox *chkAutoReconn;
    TMenuItem *ErrorDistribution1;
    TMenuItem *UniformDistribution1;
    TMenuItem *PossionDistribution1;
    TBitBtn *btnResetCount;
    TMenuItem *NoError1;
    TTrackBar *TrackBar1;
    void __fastcall About1Click(TObject *Sender);
    void __fastcall btnSaveClick(TObject *Sender);
    void __fastcall gridDevicesDblClick(TObject *Sender);
    void __fastcall gridDevicesTopLeftChanged(TObject *Sender);
    void __fastcall btnStartAllClick(TObject *Sender);
    void __fastcall btnStopAllClick(TObject *Sender);
    void __fastcall Closealldevices1Click(TObject *Sender);
    void __fastcall Openalldevices1Click(TObject *Sender);
    void __fastcall Savecurrentconfigure1Click(TObject *Sender);
    void __fastcall mnuRelodaClick(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall Exit1Click(TObject *Sender);
    void __fastcall chkActiveModeClick(TObject *Sender);
    void __fastcall mnuActivePassiveClick(TObject *Sender);
    void __fastcall DeleteItemClick(TObject *Sender);
    void __fastcall btnAddItemClick(TObject *Sender);
    void __fastcall btnBatchAddClick(TObject *Sender);
    void __fastcall chkAutoReconnClick(TObject *Sender);
    void __fastcall UniformDistribution1Click(TObject *Sender);
    void __fastcall PossionDistribution1Click(TObject *Sender);
    void __fastcall btnResetCountClick(TObject *Sender);
    void __fastcall NoError1Click(TObject *Sender);
private:	// User declarations
    bool configModified;
    map<int, WorkItem> mWorkItems;
    long seed;
//    list<TBitBtn*> btnOperation;
//    list<WorkThread*> vThreads;
    void __fastcall OperationButtonClick(TObject *Sender);
    WorkThread* __fastcall CreateWorkThread(int rowidx);

    AnsiString mHeadHex;
    AnsiString mTailHex;

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

    // Callback function from work thread
    void __fastcall onOpenChannel(WorkThread* Sender, bool opened);
    void __fastcall onCloseChannel(WorkThread* Sender, bool closed);
    void __fastcall onRxMsg(WorkThread* Sender, int msgcnt);
    void __fastcall onTxMsg(WorkThread* Sender, int msgcnt);
    void __fastcall onErrMsg(WorkThread* Sender, int msgcnt);

    void __fastcall UpdateOpenStatus(TMessage* Msg);
    void __fastcall UpdateCloseStatus(TMessage* Msg);
    void __fastcall UpdateRxMsgCnt(TMessage* Msg);
    void __fastcall UpdateTxMsgCnt(TMessage* Msg);
    void __fastcall UpdateErrMsgCnt(TMessage* Msg);
public:		// User declarations
    // Constructor
    __fastcall TFMain(TComponent* Owner);

    // Message process routine
    virtual void __fastcall Dispatch(void *Message);
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------
#endif
