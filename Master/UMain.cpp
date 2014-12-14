//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <IniFiles.hpp>

#include "UMain.h"
#include "USerialWorkThread.h"
#include "UServerWorkThread.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFMain *FMain;
//---------------------------------------------------------------------------
#define COL_IDX_SEQ          0
#define COL_IDX_MODE         1
#define COL_IDX_CONFIG       2
#define COL_IDX_SOURCE       3
#define COL_IDX_DEST         4
#define COL_IDX_TX           5
#define COL_IDX_RX           6
#define COL_IDX_CLIENT        7
#define COL_IDX_OPERATION    8

LogFileEx logger;

__fastcall TFMain::TFMain(TComponent* Owner)
        : TForm(Owner)
{
    mRxBytes = mTxBytes = 0;
    qFrontSentBytes = 0;
    
    gridDevices->ColCount = 9;
    gridDevices->RowCount = 5;
    gridDevices->Cells[COL_IDX_SEQ][0] = "Seq";
    gridDevices->Cells[COL_IDX_MODE][0] = "Mode";
    gridDevices->Cells[COL_IDX_CONFIG][0] = "Configure";
    gridDevices->Cells[COL_IDX_SOURCE][0] = "Source";
    gridDevices->Cells[COL_IDX_DEST][0] = "Destination";
    gridDevices->Cells[COL_IDX_TX][0] = "Tx(Msg)";
    gridDevices->Cells[COL_IDX_RX][0] = "Rx(Msg)";
    gridDevices->Cells[COL_IDX_CLIENT][0] = "Client";
    gridDevices->Cells[COL_IDX_OPERATION][0] = "Operation";

    gridDevices->ColWidths[COL_IDX_SEQ] = 30;
    gridDevices->ColWidths[COL_IDX_MODE] = 120;
    gridDevices->ColWidths[COL_IDX_CONFIG] = 120;
    gridDevices->ColWidths[COL_IDX_SOURCE] = 80;
    gridDevices->ColWidths[COL_IDX_DEST] = 80;
    gridDevices->ColWidths[COL_IDX_TX] = 80;
    gridDevices->ColWidths[COL_IDX_RX] = 80;
    gridDevices->ColWidths[COL_IDX_CLIENT] = 80;
    gridDevices->ColWidths[COL_IDX_OPERATION] = 80;

    lstThreadObj = new TStringList();
    csWorkVar = new TCriticalSection(); // Cretea critical section
    
    masterThread = new MasterWorkThread();
    
    ReloadConfigure();
    if (gridDevices->RowCount <= 1){
        return;
    }
    
    UpdateOperationUI();

    logger.Log("Main thread id is :" + IntToStr(::GetCurrentThreadId()));
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Exit1Click(TObject *Sender)
{
        Close();      
}
//---------------------------------------------------------------------------

void __fastcall TFMain::rbMasterClientModeClick(TObject *Sender)
{
    lblPeerIP->Enabled = true;
    txtPeerIP->Enabled = true;
    lblPeerPort->Enabled = true;
    txtPeerPort->Enabled = true;
    btnConnect->Enabled = true;
    lblConnectStatus->Enabled = true;
    chkAutoReconn->Enabled = true;
    
    lblMasterPort->Enabled = false;
    txtMasterPort->Enabled = false;
    lblPeerClientIP->Enabled = false;
    txtPeerClientIP->Enabled = false;
    btnOpen->Enabled = false;
    lblListenStatus->Enabled = false;    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::rbMasterServerModeClick(TObject *Sender)
{
    lblPeerIP->Enabled = false;
    txtPeerIP->Enabled = false;
    lblPeerPort->Enabled = false;
    txtPeerPort->Enabled = false;
    btnConnect->Enabled = false;
    lblConnectStatus->Enabled = false;
    chkAutoReconn->Enabled = false;

    lblMasterPort->Enabled = true;
    txtMasterPort->Enabled = true;
    lblPeerClientIP->Enabled = true;
    txtPeerClientIP->Enabled = true;
    btnOpen->Enabled = true;
    lblListenStatus->Enabled = true;
}
//---------------------------------------------------------------------------
void TFMain::SaveConfigure()
{
    // Save All Item list in UI
    // Using INI file to save item(User Definied)
    TIniFile *ini;
    ini = new TIniFile( ChangeFileExt(Application->ExeName, ".INI" ) );
    int devicecnt = ini->ReadInteger("head", "DeviceCount", 0);
    if (devicecnt > 0){
        // Clear old data
        for (int i = 0; i < devicecnt; i++){
            ini->EraseSection("Device" + IntToStr(i));
        }
    }
    
    devicecnt = gridDevices->RowCount - 1;
    ini->WriteInteger("head", "DeviceCount", devicecnt);
    ini->WriteBool("Master", "ClientMode", rbMasterClientMode->Checked);
    ini->WriteBool("Master", "AutoReconnect", chkAutoReconn->Checked);
    ini->WriteString("Master", "PeerIP", txtPeerIP->Text);
    ini->WriteString("Master", "PeerPort", txtPeerPort->Text);
    ini->WriteString("Master", "MasterPort", txtMasterPort->Text);
    ini->WriteString("Master", "PeerClientIP", txtPeerClientIP->Text);

    
    String sectionName;
    for (int i = 1; i <= devicecnt; i++){
        sectionName = "Device" + IntToStr(i);
        ini->WriteString(sectionName, "Mode", gridDevices->Cells[COL_IDX_MODE][i]);
        ini->WriteString(sectionName, "Configure", gridDevices->Cells[COL_IDX_CONFIG][i]);
        ini->WriteString(sectionName, "Source", gridDevices->Cells[COL_IDX_SOURCE][i]);
        ini->WriteString(sectionName, "Destination", gridDevices->Cells[COL_IDX_DEST][i]);
    }
    delete ini;
}
//---------------------------------------------------------------------------
// load and save configure
void TFMain::ReloadConfigure()
{
    // Load All Item list to UI
    // Using INI file to load item(User Definied)
    TIniFile *ini;
    ini = new TIniFile( ChangeFileExt(Application->ExeName, ".INI" ) );
    int devicecnt = ini->ReadInteger("head", "DeviceCount", 0);
    AnsiString name = ini->ReadString("head", "Name", "");
    if (name.Length() > 0){
        this->Caption = "Master - " + name;
    }
    rbMasterClientMode->Checked = ini->ReadBool("Master", "ClientMode", true);
    rbMasterServerMode->Checked = !rbMasterClientMode->Checked;
    if (rbMasterClientMode->Checked){
        rbMasterClientModeClick(rbMasterClientMode);
    }else{
        rbMasterServerModeClick(rbMasterServerMode);
    }
    
    txtPeerIP->Text = ini->ReadString("Master", "PeerIP", "");
    txtPeerPort->Text = ini->ReadString("Master", "PeerPort", "");
    txtMasterPort->Text = ini->ReadString("Master", "MasterPort", "");
    txtPeerClientIP->Text = ini->ReadString("Master", "PeerClientIP", "");
    chkAutoReconn->Checked = ini->ReadBool("Master", "AutoReconnect", false);
    
    gridDevices->RowCount = 1;
    String sectionName;
    String itemVal;
    int seq = 1;
    for (int i = 1; i <= devicecnt; i++){      
        gridDevices->RowCount++;
        sectionName = "Device" + IntToStr(i);

        itemVal = ini->ReadString(sectionName, "Mode", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Mode");
            gridDevices->RowCount--;
            continue;
        }
        gridDevices->Cells[COL_IDX_MODE][i] = itemVal;

        itemVal = ini->ReadString(sectionName, "Configure", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Configure");
            gridDevices->RowCount--;
            continue;
        }
        gridDevices->Cells[COL_IDX_CONFIG][i] = itemVal;

        itemVal = ini->ReadString(sectionName, "Source", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Source");
            gridDevices->RowCount--;
            continue;
        }
        gridDevices->Cells[COL_IDX_SOURCE][i] = itemVal;

        itemVal = ini->ReadString(sectionName, "Destination", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Destination");
            gridDevices->RowCount--;
            continue;
        }
        gridDevices->Cells[COL_IDX_DEST][i] = itemVal;

        gridDevices->Cells[COL_IDX_RX][i] = "0";
        gridDevices->Cells[COL_IDX_TX][i] = "0";
        gridDevices->Cells[COL_IDX_CLIENT][i] = "Disconnect";

        gridDevices->Cells[COL_IDX_SEQ][i] = IntToStr(seq);
        seq++;
    }
    delete ini;

    gridDevices->FixedRows = 1;
    devicecnt = gridDevices->RowCount - 1;

    try{
        csWorkVar->Enter();
        for (int i = 1; i <= devicecnt; i++){
            // create opeation button in every row
            TBitBtn *btn = new TBitBtn(this);
            btn->Tag = i; // Save row index into button tag property
            btn->Caption = "Open";
            btn->Left = gridDevices->CellRect(COL_IDX_OPERATION,i).Left + 2;
            btn->Top  = gridDevices->CellRect(COL_IDX_OPERATION,i).Top + 2;
            btn->Width = gridDevices->ColWidths[COL_IDX_OPERATION];
            btn->Height = gridDevices->RowHeights[i];
            btn->Visible = false;
            btn->Parent = this;
            btn->Glyph->LoadFromResourceID((int)HInstance, 101);
            btn->OnClick =  OperationButtonClick;

            WorkItem wi;
            wi.button = btn;
            wi.thread = CreateWorkThread(i);
            mWorkItems.insert(std::pair<int, WorkItem>(i, wi));
            lstThreadObj->AddObject(gridDevices->Cells[COL_IDX_SOURCE][i], wi.thread);
        }
    }__finally{
        csWorkVar->Leave();
    }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Saveconfigure1Click(TObject *Sender)
{
    SaveConfigure();    
}
//---------------------------------------------------------------------------
// Operation button click event
void __fastcall TFMain::OperationButtonClick(TObject *Sender)
{
    TBitBtn* button = static_cast<TBitBtn*>(Sender);

    bool stop = false;
    if (button != NULL){
        int rowidx = button->Tag;
        if ((BUTTON_STATUS_STOP & rowidx) == BUTTON_STATUS_STOP){
            rowidx &= ~BUTTON_STATUS_STOP;
            stop = true; 
        }
        // get thread from thread list
        try{
            csWorkVar->Enter();
            WorkThread* thread = mWorkItems[rowidx].thread;
            if (thread != NULL){
                if (stop){
                    thread->Stop();
                    button->Glyph->LoadFromResourceID((int)HInstance, 101);
                    button->Tag = rowidx;
                }else{
                    thread->Start();
                }
            }else{
                ShowMessage("May be parameter error, no work thread here.");
            }
        }__finally{
            csWorkVar->Leave();
        }
    }
}
//---------------------------------------------------------------------------
WorkThread* __fastcall TFMain::CreateWorkThread(int rowidx)
{
    String modeStr = gridDevices->Cells[COL_IDX_MODE][rowidx];
    String config = gridDevices->Cells[COL_IDX_CONFIG][rowidx];
    String source  = gridDevices->Cells[COL_IDX_SOURCE][rowidx];
    String dest  = gridDevices->Cells[COL_IDX_DEST][rowidx];

    // Building a new parameter
    WorkParameter param;
    param.Mode = WORK_MODE_SERIAL;
    param.Configure = config;
    param.Source = source;
    param.Destination = dest;
    param.MasterQueue = masterThread->GetQueue();

    WorkThread* thread;
    if (modeStr == "Serial port"){
        // create a new work thread
        thread = new SerialWorkThread(param);
    }else{
        // create a new server thread
        thread = new ServerWorkThread(param);

    }
    thread->Tag = rowidx;
    thread->OnOpenChannel = onOpenChannel;
    thread->OnCloseChannel = onCloseChannel;
    thread->OnRxMsg = onRxMsg;
    thread->OnTxMsg = onTxMsg;
    thread->OnServerOpen = onServerOpenChannel;
    thread->Name = source;
    thread->Resume();
    return thread;
}
//---------------------------------------------------------------------------
// Show operation button in correct row
void TFMain::UpdateOperationUI()
{
    // Update operation button position
    int startRowIdx = gridDevices->TopRow;
    TBitBtn* btn;
    try{
        csWorkVar->Enter();
        for (map<int, WorkItem>::iterator it = mWorkItems.begin();
            it != mWorkItems.end();
            ++it){
            btn = (*it).second.button;
            btn->Visible = false;
        }
        if (startRowIdx == 0){
            startRowIdx = 1; // From 1
        }
        for (int i = startRowIdx; i < startRowIdx + gridDevices->VisibleRowCount; i++){
            if ((unsigned int)i > mWorkItems.size()) break;
            btn = mWorkItems[i].button;
            if (btn != NULL){
                btn->Left = gridDevices->Left + gridDevices->CellRect(COL_IDX_OPERATION,i).Left + 2;
                btn->Top  = gridDevices->Top + gridDevices->CellRect(COL_IDX_OPERATION,i).Top + 2;
                btn->Visible = true;
            }
        }
    }__finally{
        csWorkVar->Leave();
    }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::gridDevicesTopLeftChanged(TObject *Sender)
{
    UpdateOperationUI();    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::btnOpenClick(TObject *Sender)
{
    if (btnOpen->Tag == 0){
        
        masterThread->OnRxMsg = onMasterRxMsg;
        masterThread->OnTxMsg = onMasterTxMsg;
        masterThread->OnOpenChannel = onMasterOpenChannel;
        masterThread->OnCloseChannel = onMasterCloseChannel;
        masterThread->OnServerOpen = onMasterServerOpen;
        masterThread->WorkVar = csWorkVar;
        masterThread->ThreadList = lstThreadObj;
        // open master
        masterThread->InitConnect(StrToInt(txtMasterPort->Text));
        
        masterThread->StartWorking();
        rbMasterServerMode->Enabled = false;
        rbMasterClientMode->Enabled = false;
    }else{
        masterThread->StopWorking();
        //masterThread->Suspend();

        btnOpen->Tag = 0;
        btnOpen->Glyph->LoadFromResourceID((int)HInstance, 101);
        txtPeerClientIP->Color = clWindow;

        rbMasterServerMode->Enabled = true;
        rbMasterClientMode->Enabled = true;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::ServerMasterListen(TObject *Sender,
      TCustomWinSocket *Socket)
{
    lblListenStatus->Caption = "Starting to listen...";
    btnOpen->Tag = 1;
    btnOpen->Caption = "Stop";
    btnOpen->Glyph->LoadFromResourceID((int)HInstance, 102);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::ServerMasterAccept(TObject *Sender,
      TCustomWinSocket *Socket)
{
    //lblListenStatus->Caption = "Starting to accept client connection...";
}
//---------------------------------------------------------------------------
void __fastcall TFMain::btnConnectClick(TObject *Sender)
{
    if (btnConnect->Tag == 0){
        // open
        masterThread->OnRxMsg = onMasterRxMsg;
        masterThread->OnTxMsg = onMasterTxMsg;
        masterThread->OnOpenChannel = onMasterOpenChannel;
        masterThread->OnCloseChannel = onMasterCloseChannel;
        masterThread->WorkVar = csWorkVar;
        masterThread->ThreadList = lstThreadObj;
        masterThread->InitConnect(txtPeerIP->Text, StrToInt(txtPeerPort->Text));

        masterThread->StartWorking();
        rbMasterServerMode->Enabled = false;
        rbMasterClientMode->Enabled = false;
    }else{
        masterThread->StopWorking();
        //masterThread->Suspend();
        
        btnConnect->Tag = 0;
        btnConnect->Caption = "Connect";
        btnConnect->Glyph->LoadFromResourceID((int)HInstance, 101);
        lblConnectStatus->Caption = "Wait to connect.......";

        rbMasterServerMode->Enabled = true;
        rbMasterClientMode->Enabled = true;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::tmrReconnTimer(TObject *Sender)
{
        tmrReconn->Enabled = false;
        btnConnectClick(Sender);
}
//---------------------------------------------------------------------------



void __fastcall TFMain::btnClearClick(TObject *Sender)
{
    mRxBytes = mTxBytes = 0;
    txtRxBytes->Text = "0";
    txtTxBytes->Text = "0";

    for (int i = 1; i < gridDevices->RowCount; i++){
        gridDevices->Cells[COL_IDX_RX][i] = "0";
        gridDevices->Cells[COL_IDX_TX][i] = "0";
    }
}
//---------------------------------------------------------------------------


void __fastcall TFMain::onServerOpenChannel(WorkThread* Sender, bool opened)
{
    int rowidx = Sender->Tag;

    PostMessage(Handle, WM_UPDATE_SERVER_OPEN, rowidx, opened);
}
void __fastcall TFMain::onOpenChannel(WorkThread* Sender, bool opened)
{
    int rowidx = Sender->Tag;

    PostMessage(Handle, WM_UPDATE_OPEN_STATUS, rowidx, opened);
}
void __fastcall TFMain::onCloseChannel(WorkThread* Sender, bool closed)
{
    int rowidx = Sender->Tag;
    
    PostMessage(Handle, WM_UPDATE_CLOSE_STATUS, rowidx, closed);
}
void __fastcall TFMain::onRxMsg(WorkThread* Sender, int msgcnt)
{
    int rowidx = Sender->Tag;
    
    PostMessage(Handle, WM_UPDATE_RX_MSG_CNT, rowidx, msgcnt);
}
void __fastcall TFMain::onTxMsg(WorkThread* Sender, int msgcnt)
{
    int rowidx = Sender->Tag;
    
    PostMessage(Handle, WM_UPDATE_TX_MSG_CNT, rowidx, msgcnt);
}
void __fastcall TFMain::onMasterRxMsg(MasterWorkThread* Sender, int msgcnt)
{
    PostMessage(Handle, WM_UPDATE_MASTER_RX_BYTES, 0, msgcnt);
}
void __fastcall TFMain::onMasterTxMsg(MasterWorkThread* Sender, int msgcnt)
{
    PostMessage(Handle, WM_UPDATE_MASTER_TX_BYTES, 0, msgcnt);
}
void __fastcall TFMain::UpdateOpenStatus(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    gridDevices->Cells[COL_IDX_CLIENT][rowidx] = "Connected";
}
void __fastcall TFMain::UpdateCloseStatus(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    gridDevices->Cells[COL_IDX_CLIENT][rowidx] = "Disonnected";
}
void __fastcall TFMain::UpdateRxMsgCnt(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    int msgcnt = StrToInt(gridDevices->Cells[COL_IDX_RX][rowidx]);
    msgcnt += Msg->LParam;
    gridDevices->Cells[COL_IDX_RX][rowidx] = IntToStr(msgcnt);
}
void __fastcall TFMain::UpdateTxMsgCnt(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    int msgcnt = StrToInt(gridDevices->Cells[COL_IDX_TX][rowidx]);
    msgcnt += Msg->LParam;
    gridDevices->Cells[COL_IDX_TX][rowidx] = IntToStr(msgcnt);
}

void __fastcall TFMain::UpdateMasterTxMsgCnt(TMessage* Msg)
{
    //logger.Log("Mater Tx:" + IntToStr(Msg->LParam) + "/" + txtTxBytes->Text);
    int bytes = StrToInt(txtTxBytes->Text);
    if (bytes == 0){
        mTxStartTick = ::GetTickCount();
    }
    bytes += Msg->LParam;
    txtTxBytes->Text = IntToStr(bytes);
    if (mTxStartTick > 0){
        unsigned int seconds = ((GetTickCount() - mTxStartTick)/1000);
        if (seconds > 0){
            float rate = 1.0f * bytes / seconds;
            if (rate > 1024){
                lblTxRate->Caption = IntToStr(int(rate/1024)) + "KB/S";
            }else{
                lblTxRate->Caption = IntToStr((int)rate) + "B/S";
            }
            if (seconds >= 60){
                mTxStartTick = GetTickCount();
            }
        }
    }
}
void __fastcall TFMain::UpdateMasterRxMsgCnt(TMessage* Msg)
{
    //logger.Log("Mater Rx:" + IntToStr(Msg->LParam) + "/" + txtRxBytes->Text);
    int bytes = StrToInt(txtRxBytes->Text);
    if (bytes == 0){
        mRxStartTick = ::GetTickCount();
    }
    bytes += Msg->LParam;
    txtRxBytes->Text = IntToStr(bytes);
    if (mRxStartTick > 0){
        unsigned int seconds = ((GetTickCount() - mRxStartTick)/1000);
        if (seconds > 0){
            float rate = 1.0f * bytes / seconds;
            if (rate > 1024){
                lblRxRate->Caption = IntToStr((int)(rate/1024)) + "KB/S";
            }else{
                lblRxRate->Caption = IntToStr((int)rate) + "B/S";
            }
            if (seconds >= 60){
                mTxStartTick = GetTickCount();
            }
        }
    }
}
void __fastcall TFMain::onMasterOpenChannel(WorkThread* Sender, bool opened)
{
    PostMessage(Handle, WM_UPDATE_MASTER_OPEN, 0, opened);
}
void __fastcall TFMain::onMasterCloseChannel(WorkThread* Sender, bool closed)
{
    PostMessage(Handle, WM_UPDATE_MASTER_CLOSE, 0, closed);
}
void __fastcall TFMain::onMasterServerOpen(WorkThread* Sender, bool closed)
{
    PostMessage(Handle, WM_UPDATE_MASTER_SERVER_OPEN, 0, closed);
}
void __fastcall TFMain::UpdateServerOpen(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    bool opened = Msg->LParam;

    try{
        csWorkVar->Enter();
        if (mWorkItems.find(rowidx) == mWorkItems.end()) return;

        TBitBtn* button = mWorkItems[rowidx].button;
        if (opened){
            button->Glyph->LoadFromResourceID((int)HInstance, 102);
            button->Tag = rowidx | BUTTON_STATUS_STOP;
        }else{
            ShowMessage("Open channel error in "
                + gridDevices->Cells[COL_IDX_SOURCE][rowidx]
                + "\r\n Configure:" + gridDevices->Cells[COL_IDX_CONFIG][rowidx]);
            button->Glyph->LoadFromResourceID((int)HInstance, 101);
            button->Tag = rowidx;
        }
    }__finally{
        csWorkVar->Leave();
    }    
}
void __fastcall TFMain::UpdateMasterOpenStatus(TMessage* Msg)
{
    if (rbMasterClientMode->Checked){
        btnConnect->Tag = 1;
        btnConnect->Caption = "Dis&connect";
        btnConnect->Glyph->LoadFromResourceID((int)HInstance, 102);
        lblConnectStatus->Caption = "Peer master has connected.";
    }else{
        lblListenStatus->Caption = "Client has connected.";
        txtPeerClientIP->Text = "Peer Master";
        txtPeerClientIP->Color = clMoneyGreen;
    }
}
void __fastcall TFMain::UpdateMasterCloseStatus(TMessage* Msg)
{
    if (rbMasterClientMode->Checked){
        btnConnect->Tag = 0;
        btnConnect->Caption = "&Connect";
        btnConnect->Glyph->LoadFromResourceID((int)HInstance, 101);
        lblConnectStatus->Caption = "Wait to connect.......";
    }else{
        lblListenStatus->Caption = "Client has connected.";
        txtPeerClientIP->Text = "";
        txtPeerClientIP->Color = clWindow;
    }
}
void __fastcall TFMain::UpdateMasterServerOpen(TMessage* Msg)
{
    if (rbMasterServerMode->Checked){
        lblListenStatus->Caption = "Starting to listen...";
        btnOpen->Tag = 1;
        btnOpen->Glyph->LoadFromResourceID((int)HInstance, 102);
    }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormClose(TObject *Sender, TCloseAction &Action)
{
    // Terminal all threads
    TerminateAllThread();
    
    masterThread->Terminate();
    if (!masterThread->Suspended){
        masterThread->WaitFor();
    }
    delete masterThread;
    masterThread = NULL;

    lstThreadObj->Clear();
    delete lstThreadObj;

    delete csWorkVar;
    //ShowMessage("Close");    
}
//---------------------------------------------------------------------------
void TFMain::StopAllThread()
{
    TBitBtn* button;
    try{
        csWorkVar->Enter();
        for (map<int, WorkItem>::iterator it = mWorkItems.begin();
            it != mWorkItems.end();
            ++it){
            WorkThread* thread = (*it).second.thread;
            if (thread != NULL){
                button = (*it).second.button;
                if ((button->Tag & BUTTON_STATUS_STOP) == BUTTON_STATUS_STOP){
                    thread->Stop();
                }
            }
        }
    }__finally{
        csWorkVar->Leave();
    }
}
void TFMain::TerminateAllThread()
{
    try{
        csWorkVar->Enter();
        for (map<int, WorkItem>::iterator it = mWorkItems.begin();
            it != mWorkItems.end();
            ++it){
            WorkThread* thread = (*it).second.thread;
            thread->Stop();
            thread->Terminate();
            /*if (!thread->Suspended){
                thread->WaitFor();
            }*/
            delete thread;
            (*it).second.thread = NULL;

            this->RemoveControl((*it).second.button);
            (*it).second.button->Parent = NULL;
            delete (*it).second.button;
            (*it).second.button = NULL;
        }
        mWorkItems.clear();
    }__finally{
        csWorkVar->Leave();
    }
}
//---------------------------------------------------------------------------



void __fastcall TFMain::chkAutoReconnClick(TObject *Sender)
{
    masterThread->AutoReconnect = chkAutoReconn->Checked;    
}
//---------------------------------------------------------------------------
void __fastcall TFMain::Dispatch(void *Message)
{
    switch(((PMessage)Message)->Msg)
    {
    case    WM_UPDATE_OPEN_STATUS:
        UpdateOpenStatus((TMessage*)Message);
        break;
    case    WM_UPDATE_CLOSE_STATUS:
        UpdateCloseStatus((TMessage*)Message);
        break;
    case    WM_UPDATE_RX_MSG_CNT:
        UpdateRxMsgCnt((TMessage*)Message);
        break;
    case    WM_UPDATE_TX_MSG_CNT:
        UpdateTxMsgCnt((TMessage*)Message);
        break;
    case WM_UPDATE_MASTER_RX_BYTES:
        UpdateMasterRxMsgCnt((TMessage*)Message);
        break;
    case WM_UPDATE_MASTER_TX_BYTES:
        UpdateMasterTxMsgCnt((TMessage*)Message);
        break;
    case WM_UPDATE_SERVER_OPEN:
        UpdateServerOpen((TMessage*)Message);
        break;
    case WM_UPDATE_MASTER_OPEN:
        UpdateMasterOpenStatus((TMessage*)Message);
        break;
    case WM_UPDATE_MASTER_CLOSE:
        UpdateMasterCloseStatus((TMessage*)Message);
        break;
    case WM_UPDATE_MASTER_SERVER_OPEN:
        UpdateMasterServerOpen((TMessage*)Message);
        break;
    default:
        TForm::Dispatch(Message);
        break;
    }
}
