//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <IniFiles.hpp>
#include <Classes.hpp>

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
#define COL_IDX_ERR_MODE     7
#define COL_IDX_CLIENT       8
#define COL_IDX_OPERATION    9
#define COL_IDX_MAX         10


LogFileEx logger;

__fastcall TFMain::TFMain(TComponent* Owner)
        : TForm(Owner)
{
    mRxBytes[0] = mTxBytes[0] = 0;
    mRxBytes[1] = mTxBytes[1] = 0;
    mRxBytes[2] = mTxBytes[2] = 0;
    qFrontSentBytes = 0;

    iChPri[0] = 3;
    iChPri[1] = 2;
    iChPri[2] = 1;
    UpdateChannelPriUI();
    
    CreateUI();

    lstThreadObj = new TStringList();
    csWorkVar = new TCriticalSection(); // Cretea critical section
    
    masterThread[0] = new MasterWorkThread(1, &mController, 3);
    masterThread[1] = new MasterWorkThread(2, &mController, 2);
    masterThread[2] = new MasterWorkThread(3, &mController, 1);
    
    ReloadConfigure();
    if (lstDeviceConfig.size() == 0){
        return;
    }
    
    UpdateUI();
    ReInitAllDeviceConfigure();
    UpdateOperationUI();

    logger.Log("Main thread id is :" + IntToStr(::GetCurrentThreadId()));
}
//---------------------------------------------------------------------------
void __fastcall TFMain::CreateUI()
{
    gridDevices->ColCount = COL_IDX_MAX;
    gridDevices->RowCount = 5;
    gridDevices->Cells[COL_IDX_SEQ][0] = "Seq";
    gridDevices->Cells[COL_IDX_MODE][0] = "Mode";
    gridDevices->Cells[COL_IDX_CONFIG][0] = "Configure";
    gridDevices->Cells[COL_IDX_SOURCE][0] = "Source";
    gridDevices->Cells[COL_IDX_DEST][0] = "Destination";
    gridDevices->Cells[COL_IDX_TX][0] = "Tx(Msg)";
    gridDevices->Cells[COL_IDX_RX][0] = "Rx(Msg)";
    gridDevices->Cells[COL_IDX_CLIENT][0] = "Conn Status";
    gridDevices->Cells[COL_IDX_ERR_MODE][0] = "Error Mode";
    gridDevices->Cells[COL_IDX_OPERATION][0] = "Operation";

    gridDevices->ColWidths[COL_IDX_SEQ] = 30;
    gridDevices->ColWidths[COL_IDX_MODE] = 120;
    gridDevices->ColWidths[COL_IDX_CONFIG] = 120;
    gridDevices->ColWidths[COL_IDX_SOURCE] = 60;
    gridDevices->ColWidths[COL_IDX_DEST] = 60;
    gridDevices->ColWidths[COL_IDX_TX] = 60;
    gridDevices->ColWidths[COL_IDX_RX] = 60;
    gridDevices->ColWidths[COL_IDX_CLIENT] = 80;
    gridDevices->ColWidths[COL_IDX_ERR_MODE] = 80;
    gridDevices->ColWidths[COL_IDX_OPERATION] = 80;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::ReInitAllDeviceConfigure()
{
    gridDevices->FixedRows = 1;
    int devicecnt = gridDevices->RowCount - 1;
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
void __fastcall TFMain::UpdateUI()
{
    list<device_config_t*>::const_iterator it;
    if (lstDeviceConfig.size() > 0){
        gridDevices->RowCount = lstDeviceConfig.size() + 1;
    }
    const device_config_t* pDevCfg;
    int row = 1;
    for (it = lstDeviceConfig.begin();
         it != lstDeviceConfig.end();
         ++it)
    {
        pDevCfg = *it;
        gridDevices->Cells[COL_IDX_MODE][row] = GetModeStr(pDevCfg->mode);
        gridDevices->Cells[COL_IDX_CONFIG][row] = pDevCfg->configure;
        gridDevices->Cells[COL_IDX_SOURCE][row] = pDevCfg->source;
        gridDevices->Cells[COL_IDX_DEST][row] = pDevCfg->dest;


        gridDevices->Cells[COL_IDX_RX][row] = "0";
        gridDevices->Cells[COL_IDX_TX][row] = "0";
        gridDevices->Cells[COL_IDX_ERR_MODE][row] = GetDistributionDesc(pDevCfg->errorMode);

        gridDevices->Cells[COL_IDX_SEQ][row] = pDevCfg->seq;
        row++;
    }
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
    btnOpen->Enabled = false;
    lblListenStatus->Enabled = false;
    lblChanneControl->Enabled = false;
    lblCh1->Enabled = false;
    cboErrorCH1->Enabled = false;
    cboErrorCH2->Enabled = false;
    cboErrorCH3->Enabled = false;
    cboErrorValCH1->Enabled = false;
    cboErrorValCH2->Enabled = false;
    cboErrorValCH3->Enabled = false;
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
    btnOpen->Enabled = true;
    lblListenStatus->Enabled = true;
    lblChanneControl->Enabled = true;
    lblCh1->Enabled = true;
    cboErrorCH1->Enabled = true;
    cboErrorCH2->Enabled = true;
    cboErrorCH3->Enabled = true;
    cboErrorValCH1->Enabled = true;
    cboErrorValCH2->Enabled = true;
    cboErrorValCH3->Enabled = true;
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
        for (int i = 1; i <= devicecnt; i++){
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

    ini->WriteInteger("Channel1", "ErrorMode", masterConfig[0].errorModeIdx);
    ini->WriteInteger("Channel1", "UniformValue", masterConfig[0].uniformErrorVal);
    ini->WriteInteger("Channel1", "PossionValue", masterConfig[0].possionErrorVal);
    ini->WriteInteger("Channel2", "ErrorMode", masterConfig[1].errorModeIdx);
    ini->WriteInteger("Channel2", "UniformValue", masterConfig[1].uniformErrorVal);
    ini->WriteInteger("Channel2", "PossionValue", masterConfig[1].possionErrorVal);
    ini->WriteInteger("Channel3", "ErrorMode", masterConfig[2].errorModeIdx);
    ini->WriteInteger("Channel3", "UniformValue", masterConfig[2].uniformErrorVal);
    ini->WriteInteger("Channel3", "PossionValue", masterConfig[2].possionErrorVal);

    
    String sectionName;
    list<device_config_t*>::const_iterator it;
    const device_config_t* pDevCfg;
    for (it = lstDeviceConfig.begin();
         it != lstDeviceConfig.end();
         ++it)
    {
        pDevCfg = *it;
        sectionName = "Device" + IntToStr(pDevCfg->seq);
        ini->WriteString(sectionName, "Mode", GetModeStr(pDevCfg->mode));
        ini->WriteString(sectionName, "Configure", pDevCfg->configure);
        ini->WriteString(sectionName, "Source", pDevCfg->source);
        ini->WriteString(sectionName, "Destination", pDevCfg->dest);
        ini->WriteString(sectionName, "Head", pDevCfg->head);
        ini->WriteInteger(sectionName, "Tag", pDevCfg->tag);
        ini->WriteString(sectionName, "Message", pDevCfg->message);
        ini->WriteString(sectionName, "EOFMessage", pDevCfg->eofMessage);
        ini->WriteString(sectionName, "ErrorDistribution", GetDistributionDesc(pDevCfg->errorMode));
        ini->WriteInteger(sectionName, "ErrorThreshold", pDevCfg->errorThreshold);
        ini->WriteInteger(sectionName, "MaxMessageQueue", pDevCfg->iMaxMsgQueue);
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
    chkAutoReconn->Checked = ini->ReadBool("Master", "AutoReconnect", false);
    masterConfig[0].errorModeIdx = ini->ReadInteger("Channel1", "ErrorMode", 0);
    masterConfig[0].uniformErrorVal = ini->ReadInteger("Channel1", "UniformValue", 10);
    masterConfig[0].possionErrorVal = ini->ReadInteger("Channel1", "PossionValue", 50);
    masterConfig[1].errorModeIdx = ini->ReadInteger("Channel2", "ErrorMode", 0);
    masterConfig[1].uniformErrorVal = ini->ReadInteger("Channel2", "UniformValue", 10);
    masterConfig[1].possionErrorVal = ini->ReadInteger("Channel2", "PossionValue", 50);
    masterConfig[2].errorModeIdx = ini->ReadInteger("Channel3", "ErrorMode", 0);
    masterConfig[2].uniformErrorVal = ini->ReadInteger("Channel3", "UniformValue", 10);
    masterConfig[2].possionErrorVal = ini->ReadInteger("Channel3", "PossionValue", 50);

    cboErrorCH1->ItemIndex = masterConfig[0].errorModeIdx;
    cboErrorCH2->ItemIndex = masterConfig[1].errorModeIdx;
    cboErrorCH3->ItemIndex = masterConfig[2].errorModeIdx;
    cboErrorCH1Change(NULL);
    cboErrorCH2Change(NULL);
    cboErrorCH3Change(NULL);
    
    gridDevices->RowCount = 1;
    String sectionName;
    String itemVal;
    int seq = 1;
    device_config_t* pDevCfg;
    for (int i = 1; i <= devicecnt; i++){
        pDevCfg = new device_config_t();
        
        gridDevices->RowCount++;
        sectionName = "Device" + IntToStr(i);

        itemVal = ini->ReadString(sectionName, "Mode", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Mode");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->mode = GetModeFromStr(itemVal);

        itemVal = ini->ReadString(sectionName, "Configure", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Configure");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->configure = itemVal;

        itemVal = ini->ReadString(sectionName, "Source", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Source");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->source = itemVal;

        itemVal = ini->ReadString(sectionName, "Destination", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Destination");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->dest = itemVal;

        itemVal = ini->ReadString(sectionName, "Head", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Head");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->head = itemVal;

        itemVal = ini->ReadString(sectionName, "Tag", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Tag");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->tag = StrToInt(itemVal);

        itemVal = ini->ReadString(sectionName, "Message", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Message");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->message = itemVal;

        itemVal = ini->ReadString(sectionName, "EOFMessage", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".EOFMessage");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->eofMessage = itemVal;

        itemVal = ini->ReadString(sectionName, "ErrorDistribution", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".ErrorDistribution");
            gridDevices->RowCount--;
            continue;
        }
        pDevCfg->errorMode = GetDistributionFromDesc(itemVal);
        pDevCfg->errorThreshold = ini->ReadInteger(sectionName, "ErrorThreshold", 50);

        pDevCfg->iMaxMsgQueue = ini->ReadInteger(sectionName, "MaxMessageQueue", 1);
        
        pDevCfg->seq = seq++;

        //Insert into proper position
        lstDeviceConfig.push_back(pDevCfg);
    }
    delete ini;
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
    list<device_config_t*>::const_iterator cit = lstDeviceConfig.begin();
        advance(cit, rowidx - 1); // Move ahead iSeq element
    const device_config_t* pDevCfg = *cit;

    WorkThread* thread;
    if (pDevCfg->mode == WORK_MODE_SERIAL){
        // create a new work thread
        thread = new SerialWorkThread(pDevCfg,
            pDevCfg->source, &mController);
    }else{
        // create a new server thread
        thread = new ServerWorkThread(pDevCfg,
            pDevCfg->source, &mController);

    }
    thread->Tag = rowidx;
    thread->OnOpenChannel = onOpenChannel;
    thread->OnCloseChannel = onCloseChannel;
    thread->OnRxMsg = onRxMsg;
    thread->OnTxMsg = onTxMsg;
    thread->OnServerOpen = onServerOpenChannel;
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

        for (int i = 0; i < 3; i++){
            masterThread[i]->OnRxMsg = onMasterRxMsg;
            masterThread[i]->OnTxMsg = onMasterTxMsg;
            masterThread[i]->OnOpenChannel = onMasterOpenChannel;
            masterThread[i]->OnCloseChannel = onMasterCloseChannel;
            masterThread[i]->OnServerOpen = onMasterServerOpen;
            masterThread[i]->OnTextMessage = onTextMessage;
            masterThread[i]->WorkVar = csWorkVar;
            masterThread[i]->ThreadList = lstThreadObj;
            // open master
            masterThread[i]->InitConnect(GetMasterCHPort(i));

            masterThread[i]->StartWorking();
        }

        btnOpen->Tag = 1;
        btnOpen->Caption = "&Stop";
        btnOpen->Glyph->LoadFromResourceID((int)HInstance, 102);
        lblListenStatus->Caption = "Start to open server port...";
        rbMasterServerMode->Enabled = false;
        rbMasterClientMode->Enabled = false;

        tmrReconn->Enabled = true;
    }else{
        for (int i = 0; i < 3; i++){
            masterThread[i]->StopWorking();
            SetMasterCHColor(i+1, clMedGray);
            //masterThread->Suspend();
        }

        btnOpen->Tag = 0;
        btnOpen->Caption = "&Listen";
        btnOpen->Glyph->LoadFromResourceID((int)HInstance, 101);
        lblListenStatus->Caption = "Wait to start...";
        
        rbMasterServerMode->Enabled = true;
        rbMasterClientMode->Enabled = true;

        tmrReconn->Enabled = false;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::btnConnectClick(TObject *Sender)
{
    if (btnConnect->Tag == 0){
        // open
        for (int i = 0; i < 3; i++){
            masterThread[i]->OnRxMsg = onMasterRxMsg;
            masterThread[i]->OnTxMsg = onMasterTxMsg;
            masterThread[i]->OnOpenChannel = onMasterOpenChannel;
            masterThread[i]->OnCloseChannel = onMasterCloseChannel;
            masterThread[i]->OnTextMessage = onTextMessage;
            masterThread[i]->WorkVar = csWorkVar;
            masterThread[i]->ThreadList = lstThreadObj;
            masterThread[i]->InitConnect(txtPeerIP->Text, GetPeerCHPort(i));

            masterThread[i]->StartWorking();
        }
        btnConnect->Caption = "&Stop";
        btnConnect->Tag = 1;
        lblConnectStatus->Caption = "Start to connect.......";
        btnConnect->Glyph->LoadFromResourceID((int)HInstance, 102);
        rbMasterServerMode->Enabled = false;
        rbMasterClientMode->Enabled = false;

        tmrReconn->Enabled = true;
    }else{
        for (int i = 0; i < 3; i++){
            masterThread[i]->StopWorking();
        }
        //masterThread->Suspend();
        
        btnConnect->Tag = 0;
        btnConnect->Caption = "&Start";
        btnConnect->Glyph->LoadFromResourceID((int)HInstance, 101);
        lblConnectStatus->Caption = "Wait to start.......";

        rbMasterServerMode->Enabled = true;
        rbMasterClientMode->Enabled = true;

        tmrReconn->Enabled = false;
    }
}
//---------------------------------------------------------------------------



void __fastcall TFMain::btnClearClick(TObject *Sender)
{
    mRxBytes[0] = mTxBytes[0] = 0;
    mRxBytes[1] = mTxBytes[1] = 0;
    mRxBytes[2] = mTxBytes[2] = 0;
    txtRxBytesCH1->Text = "0";
    txtTxBytesCH1->Text = "0";
    txtRxBytesCH2->Text = "0";
    txtTxBytesCH2->Text = "0";
    txtRxBytesCH3->Text = "0";
    txtTxBytesCH3->Text = "0";

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
void __fastcall TFMain::onMasterRxMsg(int ch, int msgcnt)
{
    PostMessage(Handle, WM_UPDATE_MASTER_RX_BYTES, ch, msgcnt);
}
void __fastcall TFMain::onMasterTxMsg(int ch, int msgcnt)
{
    PostMessage(Handle, WM_UPDATE_MASTER_TX_BYTES, ch, msgcnt);
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
    int ch = (int)(Msg->WParam & 0xFF);
    int bytes = GetMasterCHTxBytes(ch);
    if (bytes == 0){
        mTxStartTick = ::GetTickCount();
    }
    bytes += Msg->LParam;
    SetMasterCHTxBytes(ch, bytes);
    if (mTxStartTick > 0){
        unsigned int seconds = ((GetTickCount() - mTxStartTick)/1000);
        if (seconds > 0){
            float rate = 1.0f * bytes / seconds;
            SetTxRate(ch, rate);
            if (seconds >= 60){
                mTxStartTick = GetTickCount();
            }
        }
    }
}
void __fastcall TFMain::UpdateMasterRxMsgCnt(TMessage* Msg)
{
    int ch = (int)(Msg->WParam & 0xFF);
    //logger.Log("Mater Rx:" + IntToStr(Msg->LParam) + "/" + txtRxBytes->Text);
    char caStrBuf[100];
    int bytes = GetMasterCHRxBytes(ch);
    if (bytes == 0){
        mRxStartTick = ::GetTickCount();
    }
    bytes += Msg->LParam;
    try{
        SetMasterCHRxBytes(ch, bytes);
        if (mRxStartTick > 0){
            unsigned int seconds = ((GetTickCount() - mRxStartTick)/1000);
            if (seconds > 0){
                float rate = 1.0f * bytes / seconds;
                SetRxRate(ch, rate);
                if (seconds >= 60){
                    mTxStartTick = GetTickCount();
                }
            }
        }
    }catch(Exception& e){
    }
}
void TFMain::SetRxRate(int ch, float rate)
{
    char caStrBuf[100];
    if (rate > 1024){
        snprintf(caStrBuf, sizeof(caStrBuf), "%d KB/S", (int)(rate/1024));
    }else{
        snprintf(caStrBuf, sizeof(caStrBuf), "%d B/S", (int)(rate));
    }
    switch(ch){
    case 1:
        lblRxRate1->Caption = AnsiString(caStrBuf);
        break;
    case 2:
        lblRxRate2->Caption = AnsiString(caStrBuf);
        break;
    case 3:
        lblRxRate3->Caption = AnsiString(caStrBuf);
        break;
    }
}
void TFMain::SetTxRate(int ch, float rate)
{
    char caStrBuf[100];
    if (rate > 1024){
        snprintf(caStrBuf, sizeof(caStrBuf), "%d KB/S", (int)(rate/1024));
    }else{
        snprintf(caStrBuf, sizeof(caStrBuf), "%d B/S", (int)(rate));
    }
    switch(ch){
    case 1:
        lblTxRate1->Caption = AnsiString(caStrBuf);
        break;
    case 2:
        lblTxRate2->Caption = AnsiString(caStrBuf);
        break;
    case 3:
        lblTxRate3->Caption = AnsiString(caStrBuf);
        break;
    }
}
void __fastcall TFMain::onMasterOpenChannel(int ch, bool opened)
{
    PostMessage(Handle, WM_UPDATE_MASTER_OPEN, ch, opened);
}
void __fastcall TFMain::onMasterCloseChannel(int ch, bool closed)
{
    PostMessage(Handle, WM_UPDATE_MASTER_CLOSE, ch, closed);
}
void __fastcall TFMain::onMasterServerOpen(int ch, bool closed)
{
    PostMessage(Handle, WM_UPDATE_MASTER_SERVER_OPEN, ch, closed);
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
            button->Caption = "Close";
            button->Tag = rowidx | BUTTON_STATUS_STOP;
            gridDevices->Cells[COL_IDX_CLIENT][rowidx] = "Listened";
        }else{
            ShowMessage("Open channel error in "
                + gridDevices->Cells[COL_IDX_SOURCE][rowidx]
                + "\r\n Configure:" + gridDevices->Cells[COL_IDX_CONFIG][rowidx]);
            button->Glyph->LoadFromResourceID((int)HInstance, 101);
            button->Caption = "Open";
            button->Tag = rowidx;
        }
    }__finally{
        csWorkVar->Leave();
    }    
}

void __fastcall TFMain::onTextMessage(
    int ch, int source, AnsiString msg)
{
    switch(source){
    case TEXT_MSG_SRC_CLIENT_INFO:
        //txtPeerClientIP->Text = msg;
        break;
    case TEXT_MSG_SRC_OP_ERROR:
        lblListenStatus->Caption = msg;
        SetMasterCHColor(ch, clMedGray);

        rbMasterServerMode->Enabled = true;
        rbMasterClientMode->Enabled = true;
        break;
    case TEXT_MSG_SRC_OP_ERROR_CLIENT:
        if (rbMasterClientMode->Enabled){
            //btnConnect->Tag = 0;
            //btnConnect->Caption = "Connect";
            //btnConnect->Glyph->LoadFromResourceID((int)HInstance, 101);
            //lblConnectStatus->Caption = msg;
            btnConnectClick(NULL);
        }else{
            btnOpenClick(NULL);
        }
        break;
    case TEXT_MSG_SRC_CLIENT_NOTIFY:
        lblConnectStatus->Caption = msg;
        break;
    case TEXT_MSG_SRC_CLIENT_COUNT:
        SetMasterCHColor(ch, clGreen);
        SetMasterCHConnections(ch, msg);
        break;
    default:
        ShowMessage(msg);
    }
}

void __fastcall TFMain::UpdateMasterOpenStatus(TMessage* Msg)
{
    int ch = (int)(Msg->WParam & 0xFF);
    if (rbMasterClientMode->Checked){
        btnConnect->Tag = 1;
        //btnConnect->Caption = "Dis&connect";
        btnConnect->Glyph->LoadFromResourceID((int)HInstance, 102);
        lblConnectStatus->Caption = "Remote master has connected.";
        SetMasterCHConnections(ch, "1");
        SetMasterCHColor(ch, clGreen);
    }else{
        lblListenStatus->Caption = "Channel " + IntToStr(ch) + " has a client connected.";
        SetMasterCHColor(ch, clGreen);
    }
}
void __fastcall TFMain::UpdateMasterCloseStatus(TMessage* Msg)
{
    int ch = (int)(Msg->WParam & 0xFF);
    if (rbMasterClientMode->Checked){
        btnConnect->Tag = 0;
        //btnConnect->Caption = "&Connect";
        btnConnect->Glyph->LoadFromResourceID((int)HInstance, 101);
        lblConnectStatus->Caption = "Wait to reconnect.......";
        rbMasterServerMode->Enabled = true;
        rbMasterClientMode->Enabled = true;
        SetMasterCHConnections(ch, "0");
        SetMasterCHColor(ch, clMedGray);
    }else{
        lblListenStatus->Caption = "Client has disconnected, keep wait.";
        SetMasterCHColor(ch, clMedGray);
    }
}
void __fastcall TFMain::UpdateMasterServerOpen(TMessage* Msg)
{
    int ch = (int)(Msg->WParam & 0xFF);
    SetMasterCHColor(ch, clGreen);
    if (rbMasterServerMode->Checked){
        lblListenStatus->Caption = "Channel " + IntToStr(ch) + " starting to listen...";
    }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormClose(TObject *Sender, TCloseAction &Action)
{
    // Terminal all threads
    TerminateAllThread();
    
    /*masterThread->Terminate();
    if (!masterThread->Suspended){
        masterThread->WaitFor();
    }
    delete masterThread;
    masterThread = NULL;*/

    lstThreadObj->Clear();
    delete lstThreadObj;

    delete csWorkVar;

    device_config_t* pDevCfg;
    list<device_config_t*>::iterator it;
    for (it = lstDeviceConfig.begin();
         it != lstDeviceConfig.end();
         ++it)
    {
        pDevCfg = *it;
        delete pDevCfg;
    }
    lstDeviceConfig.clear();
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
            //thread->Stop();
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

        for (int i = 0; i < 3; i++){
            masterThread[i]->Terminate();
            masterThread[i]->WaitFor();
            delete masterThread[i];
        }
    }__finally{
        csWorkVar->Leave();
    }
}
//---------------------------------------------------------------------------



void __fastcall TFMain::chkAutoReconnClick(TObject *Sender)
{
    for (int i = 0; i < 3; i++){
        masterThread[i]->AutoReconnect = chkAutoReconn->Checked;
    }
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

int TFMain::GetMasterCHTxBytes(int ch)
{
    switch(ch){
    case 1:
        return StrToInt(txtTxBytesCH1->Text);
    case 2:
        return StrToInt(txtTxBytesCH2->Text);
    case 3:
    default:
        return StrToInt(txtTxBytesCH3->Text);
    }
}

void TFMain::SetMasterCHTxBytes(int ch, int val)
{
    AnsiString valText = IntToStr(val);
    switch(ch){
    case 1:
        txtTxBytesCH1->Text = valText;
        break;
    case 2:
        txtTxBytesCH2->Text = valText;
        break;
    case 3:
    default:
        txtTxBytesCH3->Text = valText;
    }
}

int TFMain::GetMasterCHRxBytes(int ch)
{
    switch(ch){
    case 1:
        return StrToInt(txtRxBytesCH1->Text);
    case 2:
        return StrToInt(txtRxBytesCH2->Text);
    case 3:
    default:
        return StrToInt(txtRxBytesCH3->Text);
    }
}

void TFMain::SetMasterCHRxBytes(int ch, int val)
{
    AnsiString valText = IntToStr(val);
    switch(ch){
    case 1:
        txtRxBytesCH1->Text = valText;
        break;
    case 2:
        txtRxBytesCH2->Text = valText;
        break;
    case 3:
    default:
        txtRxBytesCH3->Text = valText;
    }
}

void TFMain::SetMasterCHColor(int ch, TColor color)
{
    switch(ch){
    case 1:
        lblConnCntCH1->Color = color;
        break;
    case 2:
        lblConnCntCH2->Color = color;
        break;
    case 3:
    default:
        lblConnCntCH3->Color = color;
    }
}

void TFMain::SetMasterCHConnections(int ch, AnsiString val)
{
    switch(ch){
    case 1:
        lblConnCntCH1->Caption = val;
        break;
    case 2:
        lblConnCntCH2->Caption = val;
        break;
    case 3:
    default:
        lblConnCntCH3->Caption = val;
    }
}

short TFMain::GetMasterCHPort(int ch)
{
    int splitpos = txtMasterPort->Text.AnsiPos(",");
    if (splitpos > 0){
        // maybe like ,7000,7100,7200
        AnsiString src = txtMasterPort->Text;
        int prepos = 0;
        int splitidx = 0;
        for (int idx = 0; idx < src.Length(); idx++){
            if (src.c_str()[idx] == ','){
                AnsiString portStr = txtMasterPort->Text.SubString(prepos, idx - prepos);
                if (ch == splitidx){
                    return StrToInt(portStr);
                }
                prepos = idx;
                splitidx++;
            }
        }
    }

    // Only one port, or like 7000-, 7000-7002
    // or error port string
    int port = StrToInt(txtMasterPort->Text);
    return port + ch;
}

short TFMain::GetPeerCHPort(int ch)
{
    int splitpos = txtPeerPort->Text.AnsiPos(",");
    if (splitpos > 0){
        // maybe like ,7000,7100,7200
        AnsiString src = txtPeerPort->Text;
        int prepos = 0;
        int splitidx = 0;
        for (int idx = 0; idx < src.Length(); idx++){
            if (src.c_str()[idx] == ','){
                AnsiString portStr = txtPeerPort->Text.SubString(prepos, idx - prepos);
                if (ch == splitidx){
                    return StrToInt(portStr);
                }
                prepos = idx;
                splitidx++;
            }
        }
    }

    // Only one port, or like 7000-, 7000-7002
    // or error port string
    int port = StrToInt(txtPeerPort->Text);
    return port + ch;
}

void TFMain::UpdateChannelPriUI()
{
    int chLevel[3];
    if (iChPri[0] >= iChPri[1] && iChPri[0] >= iChPri[2]){
        chLevel[0] = 3;
    }else if(iChPri[0] < iChPri[1] && iChPri[0] < iChPri[2]){
        chLevel[0] = 1;
    }else{
        chLevel[0] = 2;
    }

    if (iChPri[1] >= iChPri[0] && iChPri[1] >= iChPri[2]){
        chLevel[1] = 3;
    }else if(iChPri[1] < iChPri[0] && iChPri[1] < iChPri[2]){
        chLevel[1] = 1;
    }else{
        chLevel[1] = 2;
    }

    if (iChPri[2] >= iChPri[0] && iChPri[2] >= iChPri[1]){
        chLevel[2] = 3;
    }else if(iChPri[2] < iChPri[0] && iChPri[2] < iChPri[1]){
        chLevel[2] = 1;
    }else{
        chLevel[2] = 2;
    }

    if (chLevel[0] == chLevel[2]) chLevel[2]--;
    if (chLevel[0] == chLevel[1]) chLevel[1]--;
    if (chLevel[1] == chLevel[2]) chLevel[2]--;



    ShapeCH12->Visible = false;
    ShapeCH13->Visible = false;
    if (chLevel[0] >= 2){ShapeCH12->Visible = true;}
    if (chLevel[0] >= 3){ShapeCH13->Visible = true;}

    ShapeCH22->Visible = false;
    ShapeCH23->Visible = false;
    if (chLevel[1] >= 2){ShapeCH22->Visible = true;}
    if (chLevel[1] >= 3){ShapeCH23->Visible = true;}

    ShapeCH32->Visible = false;
    ShapeCH33->Visible = false;
    if (chLevel[2] >= 2){ShapeCH32->Visible = true;}
    if (chLevel[2] >= 3){ShapeCH33->Visible = true;}
}

int TFMain::GetMaxPriChannel()
{
    int maxCh;
    if (iChPri[0] >= iChPri[1]){
        if (iChPri[0] >= iChPri[2]){
            maxCh = 0;
        }else{
            maxCh = 2;
        }
    }else{
        if (iChPri[1] >= iChPri[2]){
            maxCh = 1;
        }else{
            maxCh = 2;
        }
    }

    return maxCh;
}

void __fastcall TFMain::tmrReconnTimer(TObject *Sender)
{
    //Get master channel priority
    iChPri[0] = masterThread[0]->GetChannelPriority();
    iChPri[1] = masterThread[1]->GetChannelPriority();
    iChPri[2] = masterThread[2]->GetChannelPriority();
    UpdateChannelPriUI();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::cboErrorCH1Change(TObject *Sender)
{
    switch(cboErrorCH1->ItemIndex){
    case 0:
        cboErrorValCH1->Enabled = false;
        cboErrorValCH1->Text = "--";
        break;
    case 1:
        cboErrorValCH1->Enabled = true;
        cboErrorValCH1->Items->Clear();
        for (int i = 10; i <= 1000000; i = i * 10){
            cboErrorValCH1->Items->Add("1/"+IntToStr(i));
        }
        cboErrorValCH1->Text = "1/" + IntToStr(masterConfig[0].uniformErrorVal);
        break;
    case 2:
        cboErrorValCH1->Enabled = true;
        cboErrorValCH1->Items->Clear();
        for (int i = 0; i <= 100; i += 10){
            cboErrorValCH1->Items->Add(IntToStr(i)+"%");
        }
        cboErrorValCH1->Text = IntToStr(masterConfig[0].possionErrorVal) + "%";
        break;
    }
    masterConfig[0].errorModeIdx = cboErrorCH1->ItemIndex;
    cboErrorValCH1Change(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::cboErrorCH2Change(TObject *Sender)
{
    switch(cboErrorCH2->ItemIndex){
    case 0:
        cboErrorValCH2->Enabled = false;
        cboErrorValCH2->Text = "--";
        break;
    case 1:
        cboErrorValCH2->Enabled = true;
        cboErrorValCH2->Items->Clear();
        for (int i = 10; i <= 1000000; i = i * 10){
            cboErrorValCH2->Items->Add("1/"+IntToStr(i));
        }
        cboErrorValCH2->Text = "1/" + IntToStr(masterConfig[1].uniformErrorVal);
        break;
    case 2:
        cboErrorValCH2->Enabled = true;
        cboErrorValCH2->Items->Clear();
        for (int i = 0; i <= 100; i += 10){
            cboErrorValCH2->Items->Add(IntToStr(i)+"%");
        }
        cboErrorValCH2->Text = IntToStr(masterConfig[1].possionErrorVal) + "%";
        break;
    }
    masterConfig[1].errorModeIdx = cboErrorCH2->ItemIndex;
    cboErrorValCH2Change(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::cboErrorCH3Change(TObject *Sender)
{
    switch(cboErrorCH3->ItemIndex){
    case 0:
        cboErrorValCH3->Enabled = false;
        cboErrorValCH3->Text = "--";
        break;
    case 1:
        cboErrorValCH3->Enabled = true;
        cboErrorValCH3->Items->Clear();
        for (int i = 10; i <= 1000000; i = i * 10){
            cboErrorValCH3->Items->Add("1/"+IntToStr(i));
        }
        cboErrorValCH3->Text = "1/" + IntToStr(masterConfig[2].uniformErrorVal);
        break;
    case 2:
        cboErrorValCH3->Enabled = true;
        cboErrorValCH3->Items->Clear();
        for (int i = 0; i <= 100; i += 10){
            cboErrorValCH3->Items->Add(IntToStr(i)+"%");
        }
        cboErrorValCH3->Text = IntToStr(masterConfig[2].possionErrorVal) + "%";
        break;
    }
    masterConfig[2].errorModeIdx = cboErrorCH3->ItemIndex;
    cboErrorValCH3Change(NULL);
}
//---------------------------------------------------------------------------


void __fastcall TFMain::cboErrorValCH1Change(TObject *Sender)
{
    if (masterConfig[0].errorModeIdx == ERROR_MODE_UNIFORM_IDX){
        masterConfig[0].uniformErrorVal = StrToInt(
            cboErrorValCH1->Text.SubString(
                cboErrorValCH1->Text.AnsiPos("/")+1,
                cboErrorValCH1->Text.Length()));
    }else if(masterConfig[0].errorModeIdx == ERROR_MODE_POSSION_IDX){
        masterConfig[0].possionErrorVal = StrToInt(cboErrorValCH1->Text.SubString(1,
            cboErrorValCH1->Text.AnsiPos("%")-1));
    }
    UpdateChannelErrorMode(1);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::cboErrorValCH2Change(TObject *Sender)
{
    if (masterConfig[1].errorModeIdx == ERROR_MODE_UNIFORM_IDX){
        masterConfig[1].uniformErrorVal = StrToInt(
            cboErrorValCH2->Text.SubString(
                cboErrorValCH2->Text.AnsiPos("/")+1,
                cboErrorValCH2->Text.Length()));
    }else if(masterConfig[1].errorModeIdx == ERROR_MODE_POSSION_IDX){
        masterConfig[1].possionErrorVal = StrToInt(cboErrorValCH2->Text.SubString(1,
            cboErrorValCH2->Text.AnsiPos("%")-1));
    }
    UpdateChannelErrorMode(2);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::cboErrorValCH3Change(TObject *Sender)
{
    if (masterConfig[2].errorModeIdx == ERROR_MODE_UNIFORM_IDX){
        masterConfig[2].uniformErrorVal = StrToInt(
            cboErrorValCH3->Text.SubString(
                cboErrorValCH3->Text.AnsiPos("/")+1,
                cboErrorValCH3->Text.Length()));
    }else if(masterConfig[2].errorModeIdx == ERROR_MODE_POSSION_IDX){
        masterConfig[2].possionErrorVal = StrToInt(cboErrorValCH3->Text.SubString(1,
            cboErrorValCH3->Text.AnsiPos("%")-1));
    }
    UpdateChannelErrorMode(3);     
}
//---------------------------------------------------------------------------
void TFMain::UpdateChannelErrorMode(int ch)
{
    if (ch <= 0 || ch > 3) return;
    masterThread[ch-1]->UdateChannelErrorMode(&masterConfig[ch-1]);
}
