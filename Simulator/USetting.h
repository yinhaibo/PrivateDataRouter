//----------------------------------------------------------------------------
#ifndef USettingH
#define USettingH
//----------------------------------------------------------------------------
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
#include <ComCtrls.hpp>

#include "UComm.h"
#include "UWorkThread.h"
//----------------------------------------------------------------------------
class TDeviceSetting : public TForm
{
__published:
	TBevel *Bevel1;
    TLabel *Label1;
    TEdit *txtSeq;
    TLabel *Label2;
    TEdit *txtAlias;
    TLabel *Label3;
    TComboBox *cboMode;
    TEdit *txtConfigure;
    TLabel *Label4;
    TButton *btnSetting;
    TLabel *Label5;
    TLabel *Label7;
    TEdit *txtDelayFrom;
    TUpDown *udDelayFrom;
    TLabel *Label8;
    TEdit *txtDelayTo;
    TUpDown *udDelayTo;
    TBitBtn *BitBtn1;
    TBitBtn *BitBtn2;
    TLabel *Label11;
    TLabel *Label12;
    TEdit *txtRequestMessage;
    TEdit *txtResponseMessage;
    TLabel *Label13;
    TLabel *Label14;
    TLabel *Label6;
    TEdit *txtTag;
    void __fastcall btnSettingClick(TObject *Sender);
private:
    device_config_t* mpDevCfg;

    //Utils
    BYTE __fastcall transParity(char parity);
    BYTE __fastcall transStopBits(float stopbits);
    AnsiString __fastcall GetParityStr(BYTE parity);
    AnsiString __fastcall GetStopBitsStr(BYTE stopbits);
    void __fastcall SetProtocolTag(unsigned char value);
    unsigned char __fastcall GetProtocolTag();
public:
	virtual __fastcall TDeviceSetting(TComponent* AOwner);
    void __fastcall BindConfig(device_config_t* pDevCfg);

    void __fastcall InvalidUI();
    void __fastcall UpdateFromUI();
};
//----------------------------------------------------------------------------
extern PACKAGE TDeviceSetting *DeviceSetting;
//----------------------------------------------------------------------------
#endif    
