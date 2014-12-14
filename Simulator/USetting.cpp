//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "USetting.h"
#include "UCommDev_Setting.h"
#include "SocketCfg.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TDeviceSetting *DeviceSetting;
//---------------------------------------------------------------------
__fastcall TDeviceSetting::TDeviceSetting(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------


void __fastcall TDeviceSetting::SetSeq(int value)
{
    if(FSeq != value) {
        FSeq = value;
    }
}
int __fastcall TDeviceSetting::GetSeq()
{
    return FSeq;
}

void __fastcall TDeviceSetting::SetAlias(AnsiString value)
{
    if(FAlias != value) {
        FAlias = value;
    }
}
AnsiString __fastcall TDeviceSetting::GetAlias()
{
    return FAlias;
}

void __fastcall TDeviceSetting::SetMode(WorkMode value)
{
    if(FMode != value) {
        FMode = value;
    }
}
WorkMode __fastcall TDeviceSetting::GetMode()
{
    return FMode;
}

void __fastcall TDeviceSetting::SetConfigure(AnsiString value)
{
    if(FConfigure != value) {
        FConfigure = value;
    }
}
AnsiString __fastcall TDeviceSetting::GetConfigure()
{
    return FConfigure;
}

void __fastcall TDeviceSetting::SetDelayFrom(int value)
{
    if(FDelayFrom != value) {
        FDelayFrom = value;
    }
}
int __fastcall TDeviceSetting::GetDelayFrom()
{
    return FDelayFrom;
}

void __fastcall TDeviceSetting::SetDelayTo(int value)
{
    if(FDelayTo != value) {
        FDelayTo = value;
    }
}
int __fastcall TDeviceSetting::GetDelayTo()
{
    return FDelayTo;
}

void __fastcall TDeviceSetting::SetErrorFrom(int value)
{
    if(FErrorFrom != value) {
        FErrorFrom = value;
    }
}
int __fastcall TDeviceSetting::GetErrorFrom()
{
    return FErrorFrom;
}

void __fastcall TDeviceSetting::SetErrorTo(int value)
{
    if(FErrorTo != value) {
        FErrorTo = value;
    }
}
int __fastcall TDeviceSetting::GetErrorTo()
{
    return FErrorTo;
}

// Update UI from variant
void __fastcall TDeviceSetting::InvalidUI()
{
    txtSeq->Text = IntToStr(FSeq);
    txtAlias->Text = FAlias;
    cboMode->ItemIndex = (int)FMode;
    txtConfigure->Text = FConfigure;
    /*txtDelayFrom->Text = IntToStr(FDelayFrom);
    txtDelayTo->Text = IntToStr(FDelayTo);
    txtErrorFrom->Text = IntToStr(FErrorFrom);
    txtErrorTo->Text = IntToStr(FErrorTo);*/
    udDelayFrom->Position = FDelayFrom;
    udDelayTo->Position = FDelayTo;
    udErrorFrom->Position = FErrorFrom;
    udErrorTo->Position = FErrorTo;
    txtRequestMessage->Text = FRequestMsg;
    txtResponseMessage->Text = FResponseMsg;
}

// Update data from UI to variant
void __fastcall TDeviceSetting::UpdateFromUI()
{
    FSeq = StrToInt(txtSeq->Text);
    FAlias = txtAlias->Text;
    FMode = (WorkMode)cboMode->ItemIndex;
    FConfigure = txtConfigure->Text;
    /*FDelayFrom = StrToInt(txtDelayFrom->Text);
    FDelayTo = StrToInt(txtDelayTo->Text);
    FErrorFrom = StrToInt(txtErrorFrom->Text);
    FErrorTo = StrToInt(txtErrorTo->Text);*/
    FDelayFrom = udDelayFrom->Position;
    FDelayTo = udDelayTo->Position;
    FErrorFrom = udErrorFrom->Position;
    FErrorTo = udErrorTo->Position;
    FRequestMsg = txtRequestMessage->Text;
    FResponseMsg = txtResponseMessage->Text;
}

void __fastcall TDeviceSetting::btnSettingClick(TObject *Sender)
{
    if (FMode == WORK_MODE_SERIAL){
        TCommDev_Setting *setting = new TCommDev_Setting(this);

        int portidx;
        int baud;
        char parity;
        int databits;
        float stopbits;

            
        GetSerialConfigFromStr(txtConfigure->Text,
            portidx, baud,
            parity, databits, stopbits);
        setting->InitDCB(portidx,
            baud, transParity(parity),
            databits, transStopBits(stopbits));
        if (mrOk == setting->ShowModal()){
            // fill configure into txtConfigure box.
            DWORD BaudRate;
            BYTE bParity;
            BYTE bStopbits;
            BYTE ByteSize;
            BYTE StopBits;
            setting->GetDCB(portidx, BaudRate, bParity, ByteSize, StopBits);
            txtConfigure->Text = "COM" + IntToStr(portidx) + ","
                + IntToStr(BaudRate) + ","
                + GetParityStr(bParity) + ","
                + IntToStr(ByteSize) + "," + GetStopBitsStr(StopBits);
        }
        setting->Close();
        delete setting;
    }else if(FMode == WORK_MODE_TCP_CLIENT){
        TSocketSetting *setting = new TSocketSetting(this);
        AnsiString ip;
        int port;
        GetTCPClientConfigFromStr(txtConfigure->Text, ip, port);
        setting->edtIP->Text = ip;
        setting->edtPort->Text = IntToStr(port);
        if (mrOk == setting->ShowModal()){
            txtConfigure->Text = setting->edtIP->Text + ","
                + setting->edtPort->Text;
        }
        setting->Close();
        delete setting;
    }
}
//---------------------------------------------------------------------------
AnsiString __fastcall TDeviceSetting::GetParityStr(BYTE parity)
{
    /* 0-4=None,Odd,Even,Mark,Space    */
    switch(parity){
    case 0: return "N";
    case 1: return "O";
    case 2: return "E";
    case 3: return "M";
    case 4: return "S";
    default:  return "N";
    }
}
//---------------------------------------------------------------------------
AnsiString __fastcall TDeviceSetting::GetStopBitsStr(BYTE stopbits)
{
    if (stopbits == 0){
        return "1";
    }else if(stopbits == 1){
        return "1.5";
    }else{
        return "2";
    }
}
//---------------------------------------------------------------------------
BYTE __fastcall TDeviceSetting::transParity(char parity)
{
    /* 0-4=None,Odd,Even,Mark,Space    */
    switch(parity){
    case 'N': return 0;
    case 'O': return 1;
    case 'E': return 2;
    case 'M': return 3;
    case 'S': return 4;
    default:  return 0;
    }
}
//---------------------------------------------------------------------------
BYTE __fastcall TDeviceSetting::transStopBits(float stopbits)
{
    /* 0,1,2 = 1, 1.5, 2               */
    if (stopbits >= 1.8f){
        return 2;
    }else if(stopbits >= 1.2f){
        return 1;
    }else{
        return 0;
    }
}
//---------------------------------------------------------------------------


void __fastcall TDeviceSetting::SetRequestMsg(AnsiString value)
{
    if(FRequestMsg != value) {
        FRequestMsg = value;
    }
}
AnsiString __fastcall TDeviceSetting::GetRequestMsg()
{
    return FRequestMsg;
}

void __fastcall TDeviceSetting::SetResponseMsg(AnsiString value)
{
    if(FResponseMsg != value) {
        FResponseMsg = value;
    }
}
AnsiString __fastcall TDeviceSetting::GetResponseMsg()
{
    return FResponseMsg;
}


void __fastcall TDeviceSetting::SetErrorDistribution(distribution_t value)
{
    if (FDistribution != value){
        FDistribution = value;
        switch(FDistribution){
        case UNIFORM_DISTRIBUTION:
            rbUniform->Checked = true;
            break;
        case POISSON_DISTRIBUTION:
            rbPoisson->Checked = true;
            break;
        case NO_ERROR_DISTRIBUTION:
            rbNoError->Checked = true;
            break;
        }
    }
}
distribution_t __fastcall TDeviceSetting::GetErrorDistribution()
{
    return FDistribution;
}

void __fastcall TDeviceSetting::rbUniformClick(TObject *Sender)
{
    FDistribution = UNIFORM_DISTRIBUTION;
}
//---------------------------------------------------------------------------

void __fastcall TDeviceSetting::rbPoissonClick(TObject *Sender)
{
    FDistribution = POISSON_DISTRIBUTION;
}
//---------------------------------------------------------------------------

void __fastcall TDeviceSetting::rbNoErrorClick(TObject *Sender)
{
    FDistribution = NO_ERROR_DISTRIBUTION;
}
//---------------------------------------------------------------------------

