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
// Update UI from variant
void __fastcall TDeviceSetting::InvalidUI()
{
    if (mpDevCfg == NULL) return;
    txtSeq->Text = IntToStr(mpDevCfg->seq);
    txtAlias->Text = mpDevCfg->alias;
    cboMode->ItemIndex = (int)mpDevCfg->mode;
    txtConfigure->Text = mpDevCfg->configure;
    txtDelayFrom->Text = IntToStr(mpDevCfg->delayFrom);
    txtDelayTo->Text = IntToStr(mpDevCfg->delayTo);
    txtTag->Text = IntToStr(mpDevCfg->tag);
    udDelayFrom->Position = mpDevCfg->delayFrom;
    udDelayTo->Position = mpDevCfg->delayTo;
    txtRequestMessage->Text = mpDevCfg->message;
    txtResponseMessage->Text = mpDevCfg->eofMessage;
}

// Update data from UI to variant
void __fastcall TDeviceSetting::UpdateFromUI()
{
    if (mpDevCfg == NULL) return;
    mpDevCfg->alias = txtAlias->Text;
    mpDevCfg->mode = (WorkMode)cboMode->ItemIndex;
    mpDevCfg->configure = txtConfigure->Text;
    mpDevCfg->tag = StrToInt(txtTag->Text);
    mpDevCfg->delayFrom = udDelayFrom->Position;
    mpDevCfg->delayTo = udDelayTo->Position;
    mpDevCfg->message = txtRequestMessage->Text;
    mpDevCfg->eofMessage = txtResponseMessage->Text;
}

void __fastcall TDeviceSetting::BindConfig(device_config_t* pDevCfg)
{
    mpDevCfg = pDevCfg;
}

void __fastcall TDeviceSetting::btnSettingClick(TObject *Sender)
{
    if (mpDevCfg->mode == WORK_MODE_SERIAL){
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
    }else if(mpDevCfg->mode == WORK_MODE_TCP_CLIENT){
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

