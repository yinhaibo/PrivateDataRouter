//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <IniFiles.hpp>

#include "UMain.h"
#include "UAbout.h"
#include "USetting.h"
#include "LogFileEx.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "Tools.h"

TFMain *FMain;
LogFileEx logger;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define COL_IDX_SEQ             0
#define COL_IDX_ALIAS           1
#define COL_IDX_MODE            2
#define COL_IDX_CONFIG          3
#define COL_IDX_DELAY_FROM      4
#define COL_IDX_DELAY_TO        5
#define COL_IDX_TAG             6
#define COL_IDX_MESSAGE         7
#define COL_IDX_EOFMSG          8
#define COL_IDX_TX              9
#define COL_IDX_RX             10
#define COL_IDX_ERROR          11
#define COL_IDX_MSGSEQ         12
#define COL_IDX_OPERATION      13
#define COL_IDX_MAX            14

__fastcall TFMain::TFMain(TComponent* Owner)
        : TForm(Owner)
{
    configModified = false;
    // Create device UI
    CreateUI();

    logger.Log("Load simuator configure...");
    ReloadConfigure();

    if (lstDeviceConfig.size() == 0){
        return;
    }
    
    UpdateUI();
    ReInitAllDeviceConfigure();
    UpdateOperationUI();

    logger.Log("Start " + this->Caption + ".....");
}
void __fastcall TFMain::CreateUI()
{
    gridDevices->ColCount = COL_IDX_MAX;
    gridDevices->RowCount = 2;

    gridDevices->Cells[COL_IDX_SEQ][0] = "Seq";
    gridDevices->Cells[COL_IDX_ALIAS][0] = "Alias";
    gridDevices->Cells[COL_IDX_TAG][0] = "Tag";
    gridDevices->Cells[COL_IDX_MODE][0] = "Mode";
    gridDevices->Cells[COL_IDX_CONFIG][0] = "Configure";
    gridDevices->Cells[COL_IDX_DELAY_FROM][0] = "Delay from(ms)";
    gridDevices->Cells[COL_IDX_DELAY_TO][0] = "Delay to(ms)";
    gridDevices->Cells[COL_IDX_RX][0] = "Rx(Msg)";
    gridDevices->Cells[COL_IDX_TX][0] = "Tx(Msg)";
    gridDevices->Cells[COL_IDX_OPERATION][0] = "Operation";
    gridDevices->Cells[COL_IDX_ERROR][0] = "Err(Msg)";
    gridDevices->Cells[COL_IDX_MESSAGE][0] = "Message";
    gridDevices->Cells[COL_IDX_EOFMSG][0] = "EOF Message";
    gridDevices->Cells[COL_IDX_MSGSEQ][0] = "Message Seq";

    gridDevices->ColWidths[COL_IDX_SEQ] = 30;
    gridDevices->ColWidths[COL_IDX_ALIAS] = 30;
    gridDevices->ColWidths[COL_IDX_TAG] = 30;
    gridDevices->ColWidths[COL_IDX_MODE] = 60;
    gridDevices->ColWidths[COL_IDX_CONFIG] = 100;
    gridDevices->ColWidths[COL_IDX_DELAY_FROM] = 60;
    gridDevices->ColWidths[COL_IDX_DELAY_TO] = 60;
    gridDevices->ColWidths[COL_IDX_RX] = 50;
    gridDevices->ColWidths[COL_IDX_TX] = 50;
    gridDevices->ColWidths[COL_IDX_MSGSEQ] = 80;
    gridDevices->ColWidths[COL_IDX_OPERATION] = 80;
    gridDevices->ColWidths[COL_IDX_ERROR] = 50;
    gridDevices->ColWidths[COL_IDX_MESSAGE] = 80;
    gridDevices->ColWidths[COL_IDX_EOFMSG] = 80;
}

//---------------------------------------------------------------------------
WorkThread* __fastcall TFMain::CreateWorkThread(int rowidx)
{
    message_t mReqMessage;
    message_t mRespMessage;

    int iSeq = StrToInt(gridDevices->Cells[COL_IDX_SEQ][rowidx]);

    // Building a new parameter

    if (iSeq > 0 && iSeq <= (int)lstDeviceConfig.size()){
        list<device_config_t*>::iterator it = lstDeviceConfig.begin();
        advance(it, iSeq - 1); // Move ahead iSeq element
        device_config_t* pDevCfg = *it;
        WorkThread* thread;
        if (pDevCfg->mode == WORK_MODE_SERIAL){
            // create a new work thread
            thread = new SerialWorkThread(pDevCfg);
            logger.Log("Create serial work thread "
                + gridDevices->Cells[COL_IDX_ALIAS][rowidx] + ","
                + pDevCfg->configure + ", [" + IntToStr(thread->ThreadID)
                + "]");
        }else{
            // create a new client thread
            thread = new ClientWorkThread(pDevCfg);
            logger.Log("Create client tcp work thread "
                + gridDevices->Cells[COL_IDX_ALIAS][rowidx] + ","
                + pDevCfg->configure + ", [" + IntToStr(thread->ThreadID) +
                "]");
        }
        thread->Name = gridDevices->Cells[COL_IDX_ALIAS][rowidx];
        thread->Tag = rowidx;
        thread->OnOpenChannel = onOpenChannel;
        thread->OnCloseChannel = onCloseChannel;
        thread->OnRxMsg = onRxMsg;
        thread->OnTxMsg = onTxMsg;
        thread->OnErrMsg = onErrMsg;
        thread->OnMsgSeqUpdate = onMsgSeqUpdate;
        thread->ActiveMode = chkActiveMode->Checked;
        thread->MaxMessageSend = FMaxMessageSend;
        return thread;
    }else{
        return NULL;
    }
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
        WorkThread* thread = mWorkItems[rowidx].thread;
        if (thread != NULL){
            if (stop){
                logger.Log("Stop work thread "
                    + gridDevices->Cells[COL_IDX_ALIAS][rowidx]
                    + ", [" + IntToStr(thread->ThreadID) +
                    "]");
                thread->Stop();
            }else{
                logger.Log("Start work thread "
                    + gridDevices->Cells[COL_IDX_ALIAS][rowidx]
                    + ", [" + IntToStr(thread->ThreadID) +
                    "]");
                thread->Start();
            }
        }else{
            ShowMessage("May be parameter error, no work thread here.");
        }
    }
}
//---------------------------------------------------------------------------
// load and save configure
void TFMain::ReloadConfigure()
{
    // Load All Item list to UI
    // Using INI file to load item(User Definied)
    TIniFile *ini;
    ini = new TIniFile( ChangeFileExt(Application->ExeName, ".INI" ) );
    
    // Load main configure
    FName = ini->ReadString("head", "Name", "");
    if (FName.Length() > 0){
        this->Caption = "Device Simulator - " + FName;
    }
    int devicecnt = ini->ReadInteger("head", "DeviceCount", 0);
    chkActiveMode->Checked = ini->ReadBool("head", "Active", false);
    mnuActivePassive->Checked = chkActiveMode->Checked;
    chkAutoReconn->Checked = ini->ReadBool("head", "AutoReconnect", true);
    FMaxMessageSend = ini->ReadInteger("head", "MessageCount", 1000);
    txtMessageCount->Text = IntToStr(FMaxMessageSend);

    //Load device configure into lstDeviceConfig
    String sectionName;
    String itemVal;
    device_config_t* pDevCfg;
    for (int i = 1; i <= devicecnt; i++){
        pDevCfg = new device_config_t();
        pDevCfg->seq = i;

        sectionName = "Device" + IntToStr(i);
        itemVal = ini->ReadString(sectionName, "Alias", "X");
        if (itemVal == "X"){
            ShowMessage("Failure to load devices infomation from configure in "
                + sectionName + ".Alias");
            delete pDevCfg;
            continue;
        }
        pDevCfg->alias = itemVal;
        
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
        pDevCfg->head = ini->ReadString(sectionName, "Head", "DDBB");
        pDevCfg->tag = ini->ReadInteger(sectionName, "Tag", 1);
        pDevCfg->delayFrom = ini->ReadInteger(sectionName, "DelayFrom", 3000);
        pDevCfg->delayTo = ini->ReadInteger(sectionName, "DelayTo", 3000);
        
        pDevCfg->message = ini->ReadString(sectionName, "Message", "12345");
        pDevCfg->eofMessage = ini->ReadString(sectionName, "EOFMessage", "");

        //Insert into proper position
        lstDeviceConfig.push_back(pDevCfg);
    }
    delete ini;
}

void __fastcall TFMain::ReInitAllDeviceConfigure()
{
    gridDevices->FixedRows = 1;
    int devicecnt = gridDevices->RowCount - 1;
    
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
    }
}

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
    ini->WriteBool("head", "Active", chkActiveMode->Checked);
    ini->WriteBool("head", "AutoReconnect", chkAutoReconn->Checked);
    ini->WriteInteger("head", "MessageCount", FMaxMessageSend);

    String sectionName;
    int iSeq;
    list<device_config_t*>::const_iterator cit = lstDeviceConfig.begin();
    for(; cit != lstDeviceConfig.end(); ++cit){
        const device_config_t* pDevCfg = *cit;
        sectionName = "Device" + IntToStr(pDevCfg->seq);
        ini->WriteString(sectionName, "Alias", pDevCfg->alias);
        ini->WriteString(sectionName, "Mode", GetModeStr(pDevCfg->mode));
        ini->WriteString(sectionName, "Configure", pDevCfg->configure);
        ini->WriteInteger(sectionName, "DelayFrom", pDevCfg->delayFrom);
        ini->WriteInteger(sectionName, "DelayTo", pDevCfg->delayTo);
        ini->WriteString(sectionName, "Head", pDevCfg->head);
        ini->WriteInteger(sectionName, "Tag", pDevCfg->tag);
        ini->WriteString(sectionName, "Message", pDevCfg->message);
        ini->WriteString(sectionName, "EOFMessage", pDevCfg->eofMessage);
    }
    delete ini;
    configModified = false;
}

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
        gridDevices->Cells[COL_IDX_ALIAS][row] = pDevCfg->alias;
        gridDevices->Cells[COL_IDX_MODE][row] = GetModeStr(pDevCfg->mode);
        gridDevices->Cells[COL_IDX_TAG][row] = pDevCfg->tag;
        gridDevices->Cells[COL_IDX_CONFIG][row] = pDevCfg->configure;
        gridDevices->Cells[COL_IDX_DELAY_FROM][row] = pDevCfg->delayFrom;
        gridDevices->Cells[COL_IDX_DELAY_TO][row] = pDevCfg->delayTo;

        gridDevices->Cells[COL_IDX_MESSAGE][row] = pDevCfg->message;
        gridDevices->Cells[COL_IDX_EOFMSG][row] = pDevCfg->eofMessage;


        gridDevices->Cells[COL_IDX_RX][row] = "0";
        gridDevices->Cells[COL_IDX_TX][row] = "0";
        gridDevices->Cells[COL_IDX_ERROR][row] = "0";

        gridDevices->Cells[COL_IDX_SEQ][row] = pDevCfg->seq;
        row++;
    }
}
//---------------------------------------------------------------------------
// Start and stop all threads.
void TFMain::StartAllThread()
{
    TBitBtn* button;
    logger.Log("Start all threads.");
    for (map<int, WorkItem>::iterator it = mWorkItems.begin();
        it != mWorkItems.end();
        ++it){
        WorkThread* thread = (*it).second.thread;
        if (thread != NULL){
            button = (*it).second.button;
            if ((button->Tag & BUTTON_STATUS_STOP) != BUTTON_STATUS_STOP){
                thread->Start();
            }
        }
    }
}
void TFMain::StopAllThread()
{
    TBitBtn* button;
    logger.Log("Stop all threads.");
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
}
void TFMain::TerminateAllThread()
{
    logger.Log("Terminate all threads.");
    for (map<int, WorkItem>::iterator it = mWorkItems.begin();
        it != mWorkItems.end();
        ++it){
        WorkThread* thread = (*it).second.thread;
        //thread->Stop();
        //thread->Terminate();
        delete thread;
        (*it).second.thread = NULL;

        this->RemoveControl((*it).second.button);
        (*it).second.button->Parent = NULL;
        delete (*it).second.button;
        (*it).second.button = NULL;
    }
    mWorkItems.clear();
}
//---------------------------------------------------------------------------
void __fastcall TFMain::About1Click(TObject *Sender)
{
        AboutBox->Show();        
}
//---------------------------------------------------------------------------


// Double click to modify configure of device channel
void __fastcall TFMain::gridDevicesDblClick(TObject *Sender)
{
    // Double click row
    if (gridDevices->Selection.Top > 0){
        EditRow(gridDevices->Selection.Top);
    }    
}
bool TFMain::EditRow(int selRow)
{
    list<device_config_t*>::iterator it = lstDeviceConfig.begin();
    advance(it, selRow - 1); // Move ahead iSeq element
    DeviceSetting->BindConfig(*it);
    // update to setting dialog
    DeviceSetting->InvalidUI();
    if (mrOk == DeviceSetting->ShowModal()){
        configModified = true;  // Will prompt user to save configure
        DeviceSetting->UpdateFromUI();
        // Update to grid cells
        UpdateUI();

        if (mWorkItems.find(selRow) != mWorkItems.end()){
            WorkThread* thread = mWorkItems[selRow].thread;
            if (thread != NULL){
                thread->Stop();
                delete thread;
            }
            mWorkItems[selRow].thread = CreateWorkThread(selRow);
        }
        return true;
    }else{
        return false;
    }
}
//---------------------------------------------------------------------------
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
void __fastcall TFMain::onErrMsg(WorkThread* Sender, int msgcnt)
{
    int rowidx = Sender->Tag;

    PostMessage(Handle, WM_UPDATE_ERR_MSG_CNT, rowidx, msgcnt);
}
void __fastcall TFMain::onMsgSeqUpdate(WorkThread* Sender, int msgcnt)
{
    int rowidx = Sender->Tag;

    PostMessage(Handle, WM_UPDATE_MSG_SEQ, rowidx, msgcnt);
}
void __fastcall TFMain::UpdateOpenStatus(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    bool opened = Msg->LParam;

    if (mWorkItems.find(rowidx) == mWorkItems.end()) return;

    TBitBtn* button = mWorkItems[rowidx].button;
    if (opened){
        button->Glyph->LoadFromResourceID((int)HInstance, 102);
        button->Caption = "Close";
        button->Tag = rowidx | BUTTON_STATUS_STOP;
    }else{
        button->Caption = "Open";
        ShowMessage("Open channel error in "
            + gridDevices->Cells[COL_IDX_ALIAS][rowidx]
            + "\r\n Configure:" + gridDevices->Cells[COL_IDX_CONFIG][rowidx]);
        button->Glyph->LoadFromResourceID((int)HInstance, 101);
        button->Tag = rowidx;
    }

}
void __fastcall TFMain::UpdateCloseStatus(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    bool closed = Msg->LParam;
    if (mWorkItems.find(rowidx) == mWorkItems.end()) return;

    TBitBtn* button = mWorkItems[rowidx].button;
    if (closed){
        button->Glyph->LoadFromResourceID((int)HInstance, 101);
        button->Caption = "Open";
        button->Tag = rowidx;
    }else{
        ShowMessage("Open channel error in "
            + gridDevices->Cells[COL_IDX_ALIAS][rowidx]
            + "\r\n Configure:" + gridDevices->Cells[COL_IDX_CONFIG][rowidx]);
        button->Glyph->LoadFromResourceID((int)HInstance, 102);
        button->Caption = "Close";
        button->Tag = rowidx | BUTTON_STATUS_STOP;
    }

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
void __fastcall TFMain::UpdateErrMsgCnt(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    int msgcnt = StrToInt(gridDevices->Cells[COL_IDX_ERROR][rowidx]);
    msgcnt += Msg->LParam;
    gridDevices->Cells[COL_IDX_ERROR][rowidx] = IntToStr(msgcnt);
}
void __fastcall TFMain::UpdateMsgSeq(TMessage* Msg)
{
    unsigned int rowidx = Msg->WParam;
    gridDevices->Cells[COL_IDX_MSGSEQ][rowidx] = IntToStr(Msg->LParam);
}
//---------------------------------------------------------------------------
// Show operation button in correct row
void __fastcall TFMain::UpdateOperationUI()
{
    // Update operation button position
    int startRowIdx = gridDevices->TopRow;
    TBitBtn* btn;
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
            btn->Left = gridDevices->CellRect(COL_IDX_OPERATION,i).Left + 2;
            btn->Top  = gridDevices->CellRect(COL_IDX_OPERATION,i).Top + 2;
            btn->Visible = true;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::gridDevicesTopLeftChanged(TObject *Sender)
{
    UpdateOperationUI();
}
//---------------------------------------------------------------------------


void __fastcall TFMain::btnStartAllClick(TObject *Sender)
{
    StartAllThread();    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::btnStopAllClick(TObject *Sender)
{
    StopAllThread();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Closealldevices1Click(TObject *Sender)
{
    StopAllThread();    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Openalldevices1Click(TObject *Sender)
{
    StartAllThread();    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Savecurrentconfigure1Click(TObject *Sender)
{
    SaveConfigure();    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::mnuRelodaClick(TObject *Sender)
{
    palPrompt->Visible = true;
    Application->ProcessMessages();
    TerminateAllThread();
    palPrompt->Visible = false;
    ReloadConfigure();
    UpdateOperationUI();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormClose(TObject *Sender, TCloseAction &Action)
{
    if (configModified){
        if (mrYes == MessageDlg(
            "You have changed the configure of devices, would you want to save configure to file?",
            TMsgDlgType() << mtInformation,
            TMsgDlgButtons() << mbYes << mbNo,
            0)){
            SaveConfigure();
        }
    }
    palPrompt->Visible = true;
    Application->ProcessMessages();
    TerminateAllThread();
    palPrompt->Visible = false;

    SaveSimulateResult();
    //Clear data
    list<device_config_t*>::iterator it;
    for (it = lstDeviceConfig.begin();
         it != lstDeviceConfig.end();
         ++it){
         device_config_t* pDevCfg = *it;
         delete pDevCfg;
    }
    lstDeviceConfig.clear();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Exit1Click(TObject *Sender)
{
    Close();    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::chkActiveModeClick(TObject *Sender)
{
    SetAllThreadActive(chkActiveMode->Checked);
    mnuActivePassive->Checked = chkActiveMode->Checked;
}
//---------------------------------------------------------------------------
void TFMain::SetAllThreadActive(bool active)
{
    for (map<int, WorkItem>::iterator it = mWorkItems.begin();
        it != mWorkItems.end();
        ++it){
        WorkThread* thread = (*it).second.thread;
        if (thread != NULL){
            thread->ActiveMode = active;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::mnuActivePassiveClick(TObject *Sender)
{
    chkActiveMode->Checked = mnuActivePassive->Checked;
    SetAllThreadActive(chkActiveMode->Checked);   
}
//---------------------------------------------------------------------------

void __fastcall TFMain::DeleteItemClick(TObject *Sender)
{
    AnsiString deleteAlias;
    for (int i = gridDevices->Selection.Top;
        i <= gridDevices->Selection.Bottom;
        i++){
        if (deleteAlias.Length() > 0){
            deleteAlias = deleteAlias + ",";
        }
        deleteAlias += gridDevices->Cells[COL_IDX_ALIAS][i];
    }
    if (mrYes == MessageDlg("Do you want to delete ("
        + deleteAlias + ") device?",
        mtInformation,
        TMsgDlgButtons() << mbYes << mbNo, 0)){
        // stop and delete device
        int offset = gridDevices->Selection.Bottom - gridDevices->Selection.Top + 1;
        for (int i = gridDevices->Selection.Top;
        i <= gridDevices->Selection.Bottom;
        i++){
            if (gridDevices->RowCount <= (i + offset)){
                //Empty line
                if (mWorkItems[i].thread != NULL){
                    mWorkItems[i].thread->Stop();
                    mWorkItems[i].thread->Terminate();
                    delete mWorkItems[i].thread;
                }
                if (mWorkItems[i].button != NULL){
                    this->RemoveControl(mWorkItems[i].button);
                    mWorkItems[i].button->Parent = NULL;
                    delete mWorkItems[i].button;
                }
                mWorkItems.erase(i);
            }else{
                //Replace line
                for (int j = 1; j < gridDevices->ColCount; j++){
                    gridDevices->Cells[j][i] = gridDevices->Cells[j][i+offset];
                    gridDevices->Cells[j][i+offset] = "";
                }
                if (mWorkItems[i].thread != NULL){
                    mWorkItems[i].thread->Stop();
                    mWorkItems[i].thread->Terminate();
                    delete mWorkItems[i].thread;
                }
                if (mWorkItems[i].button != NULL){
                    this->RemoveControl(mWorkItems[i].button);
                    mWorkItems[i].button->Parent = NULL;
                    delete mWorkItems[i].button;
                }
                mWorkItems[i] = mWorkItems[i+offset];
                mWorkItems.erase(i+offset);
            }
        }
        gridDevices->RowCount -= offset;
        UpdateOperationUI();
    }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::btnAddItemClick(TObject *Sender)
{
    int cpyrow;
    gridDevices->RowCount++;
    int rowidx = gridDevices->RowCount - 1;
    gridDevices->Cells[COL_IDX_SEQ][rowidx] = IntToStr(rowidx);
    if (rowidx > 1){
        cpyrow = rowidx - 1;
        if (gridDevices->Selection.Top > 1){
            cpyrow = gridDevices->Selection.Top;
        }
        for (int j = 1; j < gridDevices->ColCount; j++){
            gridDevices->Cells[j][rowidx] = gridDevices->Cells[j][cpyrow];
        }
        
        AnsiString aliasIdxStr = "000" + IntToStr(StrToInt(
                gridDevices->Cells[COL_IDX_ALIAS][rowidx].SubString(2,3)
                )+1);
        aliasIdxStr = aliasIdxStr.SubString(aliasIdxStr.Length()-3+1, 3);
        gridDevices->Cells[COL_IDX_ALIAS][rowidx] =
            gridDevices->Cells[COL_IDX_ALIAS][rowidx].SubString(1,1)
            + aliasIdxStr;
        if (GetModeFromStr(gridDevices->Cells[COL_IDX_MODE][rowidx]) == WORK_MODE_SERIAL){
            // Increment COM port idx
            aliasIdxStr = gridDevices->Cells[COL_IDX_CONFIG][rowidx];
            aliasIdxStr = "COM" + IntToStr(StrToInt(
                aliasIdxStr.SubString(4,aliasIdxStr.AnsiPos(",") - 4)
                )+1);
            gridDevices->Cells[COL_IDX_CONFIG][rowidx] = aliasIdxStr +
                gridDevices->Cells[COL_IDX_CONFIG][rowidx].SubString(
                    gridDevices->Cells[COL_IDX_CONFIG][rowidx].AnsiPos(","),
                    gridDevices->Cells[COL_IDX_CONFIG][rowidx].Length());
        }else if(GetModeFromStr(gridDevices->Cells[COL_IDX_MODE][rowidx]) == WORK_MODE_TCP_CLIENT){
            // Increment TCP port idx
            aliasIdxStr = gridDevices->Cells[COL_IDX_CONFIG][rowidx];
            aliasIdxStr = IntToStr(StrToInt(
                aliasIdxStr.SubString(aliasIdxStr.AnsiPos(",")+1, aliasIdxStr.Length())
                )+1);
            gridDevices->Cells[COL_IDX_CONFIG][rowidx] =
                gridDevices->Cells[COL_IDX_CONFIG][rowidx].SubString(1, gridDevices->Cells[COL_IDX_CONFIG][rowidx].AnsiPos(","))
                + aliasIdxStr;
        }
    }else{
        gridDevices->Cells[COL_IDX_ALIAS][rowidx] = "A001";
        gridDevices->Cells[COL_IDX_MODE][rowidx] = GetModeStr(WORK_MODE_SERIAL);
        gridDevices->Cells[COL_IDX_CONFIG][rowidx] = "COM1,115200,N,8,1";
        gridDevices->Cells[COL_IDX_DELAY_FROM][rowidx] = "4000";
        gridDevices->Cells[COL_IDX_DELAY_TO][rowidx] = "4000";
        gridDevices->Cells[COL_IDX_RX][rowidx] = "0";
        gridDevices->Cells[COL_IDX_TX][rowidx] = "0";
        gridDevices->Cells[COL_IDX_ERROR][rowidx] = "0";
        gridDevices->FixedRows = 1;
    }
    EditRow(rowidx);
    // create opeation button in every row
    TBitBtn *btn = new TBitBtn(this);
    btn->Tag = rowidx; // Save row index into button tag property
    btn->Caption = "Open";
    btn->Left = gridDevices->CellRect(COL_IDX_OPERATION,rowidx).Left + 2;
    btn->Top  = gridDevices->CellRect(COL_IDX_OPERATION,rowidx).Top + 2;
    btn->Width = gridDevices->ColWidths[COL_IDX_OPERATION];
    btn->Height = gridDevices->RowHeights[rowidx];
    btn->Visible = false;
    btn->Parent = this;
    btn->Glyph->LoadFromResourceID((int)HInstance, 101);
    btn->OnClick =  OperationButtonClick;

    WorkItem wi;
    wi.button = btn;
    wi.thread = CreateWorkThread(rowidx);
    mWorkItems.insert(std::pair<int, WorkItem>(rowidx, wi));
    UpdateOperationUI();
    gridDevices->Selection.Top = rowidx;
    gridDevices->Selection.Bottom = rowidx+1;
    TGridRect myRect;
    myRect.Left = 1;
    myRect.Top = rowidx;
    myRect.Right = COL_IDX_OPERATION;
    myRect.Bottom = rowidx;
    gridDevices->Selection = myRect;

}
//---------------------------------------------------------------------------

void __fastcall TFMain::btnBatchAddClick(TObject *Sender)
{
    int cpyrow;
    AnsiString batch = InputBox("Add number:", "Batch add number", "5");
    if (batch > 0){
        for (int i = 1; i <= batch; i++){
            gridDevices->RowCount++;
            int rowidx = gridDevices->RowCount - 1;
            gridDevices->Cells[COL_IDX_SEQ][rowidx] = IntToStr(rowidx);
            if (rowidx > 1){
                cpyrow = rowidx - 1;
                if (gridDevices->Selection.Top > 1){
                    cpyrow = gridDevices->Selection.Top;
                }
                for (int j = 1; j < gridDevices->ColCount; j++){
                    gridDevices->Cells[j][rowidx] = gridDevices->Cells[j][cpyrow];
                }
                AnsiString aliasIdxStr = "000" + IntToStr(StrToInt(
                        gridDevices->Cells[COL_IDX_ALIAS][rowidx].SubString(2,3)
                        )+1);
                aliasIdxStr = aliasIdxStr.SubString(aliasIdxStr.Length()-3+1, 3);
                gridDevices->Cells[COL_IDX_ALIAS][rowidx] =
                    gridDevices->Cells[COL_IDX_ALIAS][rowidx].SubString(1,1)
                    + aliasIdxStr;
                if (GetModeFromStr(gridDevices->Cells[COL_IDX_MODE][rowidx]) == WORK_MODE_SERIAL){
                    // Increment COM port idx
                    aliasIdxStr = gridDevices->Cells[COL_IDX_CONFIG][rowidx];
                    aliasIdxStr = "COM" + IntToStr(StrToInt(
                        aliasIdxStr.SubString(4,aliasIdxStr.AnsiPos(",") - 4)
                        )+1);
                    gridDevices->Cells[COL_IDX_CONFIG][rowidx] = aliasIdxStr +
                        gridDevices->Cells[COL_IDX_CONFIG][rowidx].SubString(
                            gridDevices->Cells[COL_IDX_CONFIG][rowidx].AnsiPos(","),
                            gridDevices->Cells[COL_IDX_CONFIG][rowidx].Length());
                }else if(GetModeFromStr(gridDevices->Cells[COL_IDX_MODE][rowidx]) == WORK_MODE_TCP_CLIENT){
                    // Increment TCP port idx
                    aliasIdxStr = gridDevices->Cells[COL_IDX_CONFIG][rowidx];
                    aliasIdxStr = IntToStr(StrToInt(
                        aliasIdxStr.SubString(aliasIdxStr.AnsiPos(",")+1, aliasIdxStr.Length())
                        )+1);
                    gridDevices->Cells[COL_IDX_CONFIG][rowidx] =
                        gridDevices->Cells[COL_IDX_CONFIG][rowidx].SubString(1, gridDevices->Cells[COL_IDX_CONFIG][rowidx].AnsiPos(","))
                        + aliasIdxStr;
                }
            }else{
                gridDevices->Cells[COL_IDX_ALIAS][rowidx] = "A000";
                gridDevices->Cells[COL_IDX_MODE][rowidx] = GetModeStr(WORK_MODE_SERIAL);
                gridDevices->Cells[COL_IDX_CONFIG][rowidx] = "COM1,115200,N,8,1";
                gridDevices->Cells[COL_IDX_DELAY_FROM][rowidx] = "4000";
                gridDevices->Cells[COL_IDX_DELAY_TO][rowidx] = "4000";
                gridDevices->Cells[COL_IDX_RX][rowidx] = "0";
                gridDevices->Cells[COL_IDX_TX][rowidx] = "0";
                gridDevices->Cells[COL_IDX_ERROR][rowidx] = "0";

            }

            // create opeation button in every row
            TBitBtn *btn = new TBitBtn(this);
            btn->Tag = rowidx; // Save row index into button tag property
            btn->Caption = "Open";
            btn->Left = gridDevices->CellRect(COL_IDX_OPERATION,rowidx).Left + 2;
            btn->Top  = gridDevices->CellRect(COL_IDX_OPERATION,rowidx).Top + 2;
            btn->Width = gridDevices->ColWidths[COL_IDX_OPERATION];
            btn->Height = gridDevices->RowHeights[rowidx];
            btn->Visible = false;
            btn->Parent = this;
            btn->Glyph->LoadFromResourceID((int)HInstance, 101);
            btn->OnClick =  OperationButtonClick;

            WorkItem wi;
            wi.button = btn;
            wi.thread = CreateWorkThread(rowidx);
            mWorkItems.insert(std::pair<int, WorkItem>(rowidx, wi));

            TGridRect myRect;
            myRect.Left = 1;
            myRect.Top = rowidx;
            myRect.Right = COL_IDX_OPERATION;
            myRect.Bottom = rowidx;
            gridDevices->Selection = myRect;

            gridDevices->TopRow = gridDevices->RowCount - gridDevices->VisibleRowCount;
        }
        gridDevices->FixedRows = 1;
        UpdateOperationUI();
    }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::chkAutoReconnClick(TObject *Sender)
{
    for (map<int, WorkItem>::iterator it = mWorkItems.begin();
        it != mWorkItems.end();
        ++it){
        WorkThread* thread = (*it).second.thread;
        if (thread != NULL){
            thread->setReconnect(chkAutoReconn->Checked);
        }
    }    
}
//---------------------------------------------------------------------------



void __fastcall TFMain::btnResetCountClick(TObject *Sender)
{
    for (int rowidx = 1; rowidx < gridDevices->RowCount; rowidx++){
        gridDevices->Cells[COL_IDX_RX][rowidx] = "0";
        gridDevices->Cells[COL_IDX_TX][rowidx] = "0";
        gridDevices->Cells[COL_IDX_ERROR][rowidx] = "0";
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
    case WM_UPDATE_ERR_MSG_CNT:
        UpdateErrMsgCnt((TMessage*)Message);
        break;
    case WM_UPDATE_MSG_SEQ:
        UpdateMsgSeq((TMessage*)Message);
        break;
    default:
        TForm::Dispatch(Message);
        break;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::SaveSimulateResult()
{
    txtResult->Lines->Clear();
    AnsiString result;
    char formatBuf[512];
    snprintf(formatBuf, sizeof(formatBuf),
        "%10s %10s %10s %10s %10s %26s %26s\r\n",
        "","","","","",
        "   Response Time(ms)  ",
        "   Re-send Count      ");
    result = formatBuf;
    snprintf(formatBuf, sizeof(formatBuf),
        "%10s %10s %10s %10s %10s %8s %8s %8s %8s %8s %8s\r\n",
        " Device ",
        " Message ",
        "   Send  ",
        " Receive ",
        "   Error ",
        " Min", " Max", " Avg",
        " Min", " Max", " Avg");
    result += formatBuf;
    snprintf(formatBuf, sizeof(formatBuf),
        "%s %s %s\r\n",
        "---------- ---------- ---------- ---------- ----------",
        "-------- -------- -------- --------",
        "-------- --------");
    result += formatBuf;
    list<device_config_t*>::const_iterator cit = lstDeviceConfig.begin();
    for(; cit != lstDeviceConfig.end(); ++cit){
        const device_config_t* pDevCfg = *cit;
        snprintf(formatBuf, sizeof(formatBuf),
            "%10s %10d %10d %10d %10d %8d %8d %8.2f %8d %8d %8.2f\r\n",
            pDevCfg->alias.c_str(), pDevCfg->msgMsgSent,
            pDevCfg->msgTxCnt, pDevCfg->msgRxCnt, pDevCfg->msgErrCnt,
            pDevCfg->dcRespTime.min, pDevCfg->dcRespTime.max, pDevCfg->dcRespTime.avg,
            pDevCfg->dcResendCnt.min, pDevCfg->dcResendCnt.max, pDevCfg->dcResendCnt.avg
        );
        result += formatBuf;
    }
    txtResult->Text = result;
    txtResult->Lines->SaveToFile("Simulator" + FName + "_Result.txt");
}
void __fastcall TFMain::txtResultDblClick(TObject *Sender)
{
    txtResult->Visible = false;    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::btnResultClick(TObject *Sender)
{
    SaveSimulateResult();
    txtResult->Visible = true;
    txtResult->BringToFront();    
}
//---------------------------------------------------------------------------

void __fastcall TFMain::tmrWriteResultTimer(TObject *Sender)
{
    SaveSimulateResult();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::txtMessageCountChange(TObject *Sender)
{
    FMaxMessageSend = StrToInt(txtMessageCount->Text);
    for (map<int, WorkItem>::iterator it = mWorkItems.begin();
        it != mWorkItems.end();
        ++it){
        WorkThread* thread = (*it).second.thread;
        if (thread != NULL){
            thread->MaxMessageSend = FMaxMessageSend;
        }
    }
}
//---------------------------------------------------------------------------

