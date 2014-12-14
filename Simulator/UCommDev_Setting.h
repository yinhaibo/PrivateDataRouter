//---------------------------------------------------------------------------

#ifndef UCommDev_SettingH
#define UCommDev_SettingH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TCommDev_Setting : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *GBoxBuffer;
	TLabel *lbInbs;
	TLabel *lbOutbs;
	TEdit *EditInBufSize;
	TEdit *EditOutBufSize;
	TBitBtn *BnDefault;
	TBitBtn *BnOK;
	TBitBtn *BnCancel;
	TGroupBox *GboxBase;
	TLabel *lbPort;
	TLabel *lbBaud;
	TLabel *lbCheck;
	TLabel *lbBits;
	TLabel *lbStops;
	TComboBox *CbStopBits;
	TComboBox *CbSelBaud;
	TComboBox *CbParity;
	TComboBox *CbByteSize;
	TComboBox *CbPort;
	TGroupBox *GBoxModem;
	TLabel *lbFlow;
	TLabel *lbAAns;
	TComboBox *CbFlow;
	TEdit *EditAutoAns;
	void __fastcall BnDefaultClick(TObject *Sender);
	void __fastcall BnCancelClick(TObject *Sender);
	void __fastcall BnOKClick(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
private:
	bool FIsValid;	// User declarations

    void __fastcall LoadDefaultSetting();
public:		// User declarations
	__fastcall TCommDev_Setting(TComponent* Owner);
	bool __fastcall GetIsValid();
    void InitDCB(const int portidx, const DCB *pdcb);
    void InitDCB(const int portidx, const DCB *pdcb,
        const DWORD inSize, const DWORD outSize);
    void InitDCB(const int portidx,
        const DWORD BaudRate,       /* Baudrate at which running       */
        const BYTE Parity,          /* 0-4=None,Odd,Even,Mark,Space    */
        const BYTE ByteSize,        /* Number of bits/byte, 4-8        */
        const BYTE StopBits        /* 0,1,2 = 1, 1.5, 2               */
    );

    void GetDCB(int& portidx,
        DWORD& BaudRate,       /* Baudrate at which running       */
        BYTE& Parity,          /* 0-4=None,Odd,Even,Mark,Space    */
        BYTE& ByteSize,        /* Number of bits/byte, 4-8        */
        BYTE& StopBits        /* 0,1,2 = 1, 1.5, 2               */);
    
    int SetDCB(DCB *pdcb);
    int SetDCB(DCB *pdcb, DWORD* pinSize, DWORD *poutSize);
public:
	__property bool IsValid  = { read=GetIsValid};
};
//---------------------------------------------------------------------------
class PACKAGE TCommSerialPortInfo
 {
   public:
     __property TStringList *PortList = { read = _PortList };
     void __fastcall Refresh(void);

     __fastcall TCommSerialPortInfo();
     virtual __fastcall ~TCommSerialPortInfo();

     __property bool IsFromSystem = { read = _bFromSysDrv };
     static int __fastcall PortNo(AnsiString s);
     static AnsiString __fastcall PortName(int iPortNo);

   private:
     static int __fastcall PortListSortCompare(TStringList* lpList, int Index1, int Index2);

     TStringList *_PortList;
     bool _bFromSysDrv;
     void __fastcall ListFromSystem(void);
 };
#endif
