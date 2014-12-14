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
    TLabel *Label6;
    TLabel *Label7;
    TEdit *txtDelayFrom;
    TUpDown *udDelayFrom;
    TLabel *Label8;
    TEdit *txtDelayTo;
    TUpDown *udDelayTo;
    TUpDown *udErrorTo;
    TEdit *txtErrorTo;
    TLabel *Label9;
    TUpDown *udErrorFrom;
    TEdit *txtErrorFrom;
    TLabel *Label10;
    TBitBtn *BitBtn1;
    TBitBtn *BitBtn2;
    TLabel *Label11;
    TLabel *Label12;
    TEdit *txtRequestMessage;
    TEdit *txtResponseMessage;
    TLabel *Label13;
    TLabel *Label14;
    TLabel *Label15;
    TRadioButton *rbUniform;
    TRadioButton *rbPoisson;
    TRadioButton *rbNoError;
    void __fastcall btnSettingClick(TObject *Sender);
    void __fastcall rbUniformClick(TObject *Sender);
    void __fastcall rbPoissonClick(TObject *Sender);
    void __fastcall rbNoErrorClick(TObject *Sender);
private:
    int FSeq;
    AnsiString FAlias;
    WorkMode FMode;
    AnsiString FConfigure;
    int FDelayFrom;
    int FDelayTo;
    int FErrorFrom;
    int FErrorTo;
    AnsiString FRequestMsg;
    AnsiString FResponseMsg;
    distribution_t FDistribution;

    void __fastcall SetSeq(int value);
    int __fastcall GetSeq();
    void __fastcall SetAlias(AnsiString value);
    AnsiString __fastcall GetAlias();
    void __fastcall SetMode(WorkMode value);
    WorkMode __fastcall GetMode();
    void __fastcall SetConfigure(AnsiString value);
    AnsiString __fastcall GetConfigure();
    void __fastcall SetDelayFrom(int value);
    int __fastcall GetDelayFrom();
    void __fastcall SetDelayTo(int value);
    int __fastcall GetDelayTo();
    void __fastcall SetErrorFrom(int value);
    int __fastcall GetErrorFrom();
    void __fastcall SetErrorTo(int value);
    int __fastcall GetErrorTo();
    void __fastcall SetRequestMsg(AnsiString value);
    AnsiString __fastcall GetRequestMsg();
    void __fastcall SetResponseMsg(AnsiString value);
    AnsiString __fastcall GetResponseMsg();

    //Utils
    BYTE __fastcall transParity(char parity);
    BYTE __fastcall transStopBits(float stopbits);
    AnsiString __fastcall GetParityStr(BYTE parity);
    AnsiString __fastcall GetStopBitsStr(BYTE stopbits);
    void __fastcall SetErrorDistribution(distribution_t value);
    distribution_t __fastcall GetErrorDistribution();
public:
	virtual __fastcall TDeviceSetting(TComponent* AOwner);
    __property int Seq  = { read=GetSeq, write=SetSeq };
    __property AnsiString Alias  = { read=GetAlias, write=SetAlias };
    __property WorkMode Mode  = { read=GetMode, write=SetMode };
    __property AnsiString Configure  = { read=GetConfigure, write=SetConfigure };
    __property int DelayFrom  = { read=GetDelayFrom, write=SetDelayFrom };
    __property int DelayTo  = { read=GetDelayTo, write=SetDelayTo };
    __property int ErrorFrom  = { read=GetErrorFrom, write=SetErrorFrom };
    __property int ErrorTo  = { read=GetErrorTo, write=SetErrorTo };

    void __fastcall InvalidUI();
    void __fastcall UpdateFromUI();
    __property AnsiString RequestMsg  = { read=GetRequestMsg, write=SetRequestMsg };
    __property AnsiString ResponseMsg  = { read=GetResponseMsg, write=SetResponseMsg };
    __property distribution_t ErrorDistribution  = { read=GetErrorDistribution, write=SetErrorDistribution };
};
//----------------------------------------------------------------------------
extern PACKAGE TDeviceSetting *DeviceSetting;
//----------------------------------------------------------------------------
#endif    
