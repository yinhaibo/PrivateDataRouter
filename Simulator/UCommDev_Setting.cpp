#include <vcl.h>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <TabNotBk.hpp>
#include <Tabs.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
#include <ADODB.hpp>
#include <DB.hpp>
#include <msxmldom.hpp>
#include <XMLDoc.hpp>
#include <xmldom.hpp>
#include <XMLIntf.hpp>
#include <jpeg.hpp>
#include <Registry.hpp>

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#pragma hdrstop

#pragma hdrstop

#include "UCommDev_Setting.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
/***************************************************************************\
*                           TCommSerialPortInfo                             *
\***************************************************************************/

__fastcall TCommSerialPortInfo::TCommSerialPortInfo()
{
  _bFromSysDrv = false;
  _PortList = new TStringList;
  Refresh();
}
//---------------------------------------------------------------------------

__fastcall TCommSerialPortInfo::~TCommSerialPortInfo()
{
  delete _PortList;
}
//---------------------------------------------------------------------------

void __fastcall TCommSerialPortInfo::Refresh(void)
{
  _PortList->Clear();
  _bFromSysDrv = false;
  ListFromSystem();

  if(_PortList->Count<=0)
   {
     for(int i=1; i<=4; i++)
       _PortList->Add(PortName(i));
     _bFromSysDrv = false;
   }
}
//---------------------------------------------------------------------------

int __fastcall TCommSerialPortInfo::PortNo(AnsiString s)
{
  if(s.SubString(1,3).UpperCase()=="COM")
   {
     int n = atoi(s.SubString(4,s.Length()).c_str());
     if(n>0)return n;
   }
  return 0;
}
//---------------------------------------------------------------------------

AnsiString __fastcall TCommSerialPortInfo::PortName(int iPortNo)
{
  AnsiString s;
  s.sprintf("COM%d",iPortNo);
  return s;
}
//---------------------------------------------------------------------------

void __fastcall TCommSerialPortInfo::ListFromSystem(void)
{
  TRegistry *reg=NULL;
  TStringList *names=NULL;
  AnsiString sPName;
  int iPortNo;

  try
   {
     try
      {
        names = new TStringList;
        reg = new TRegistry;

        reg->RootKey=HKEY_LOCAL_MACHINE;
        reg->OpenKey("HARDWARE\\DEVICEMAP\\SERIALCOMM",false);
        reg->GetValueNames(names);

        for(int i=0; i<names->Count; i++)
         {
           try
            {
              sPName = reg->ReadString(names->Strings[i]);
              iPortNo = PortNo(sPName);
              if(iPortNo>0)
               {
                 _PortList->Add(PortName(iPortNo));
               }
            }
           catch(Exception &e)
            {
              //ignore errors
            }
         }

        _PortList->CustomSort(PortListSortCompare);
        _bFromSysDrv = true;
      }
     catch(Exception &e)
      {
        //ignore errors
      }
   }
  __finally
   {
     if(reg) delete reg;
     if(names) delete names;
   }
}

//---------------------------------------------------------------------------
// TCommDev_Setting From
//---------------------------------------------------------------------------
__fastcall TCommDev_Setting::TCommDev_Setting(TComponent* Owner)
	: TForm(Owner)
{
	TCommSerialPortInfo spi;
	CbPort->Clear();
  	for(int i=0; i<spi.PortList->Count; i++)
		CbPort->Items->Add(spi.PortList->Strings[i]);

  	CbParity->Clear();
  	CbParity->Items->Add("No parity");
  	CbParity->Items->Add("Odd");
  	CbParity->Items->Add("Even");
  	CbParity->Items->Add("Mark");
  	CbParity->Items->Add("Space");

  	CbFlow->Clear();
  	CbFlow->Items->Add("None"); //None
  	CbFlow->Items->Add("RTS/CTS"); //"RTS/CTS "
  	CbFlow->Items->Add("Xon/Xoff"); //"Xon/Xoff"
  	CbFlow->Items->Add("RTS/CTS & Xon/Xoff");

        LoadDefaultSetting();
}

//用DCB结构数据初始化缺省数据
void TCommDev_Setting::InitDCB(const int portidx, const DCB *pdcb)
{
	InitDCB(portidx, pdcb, 1024, 1024);
}

void TCommDev_Setting::InitDCB(const int portidx, const DCB *pdcb,
        	const DWORD inSize, const DWORD outSize)
{
	int idx;
        for (idx = 0; idx < CbPort->Items->Count; idx++){
        	if (StrToInt(CbPort->Items->operator [](idx).SubString(4,3)) == portidx)
                {
                	CbPort->ItemIndex = idx;
                        break;
                }
        }
        CbPort->Text = "COM" + IntToStr(portidx);

        for (idx = 0; idx < CbSelBaud->Items->Count; idx++){
        	if ((DWORD)StrToInt(CbSelBaud->Items->operator [](idx) ) == pdcb->BaudRate){
                	CbSelBaud->ItemIndex = idx;
                        break;
                }
        }
	CbParity->ItemIndex = pdcb->Parity;
	CbByteSize->ItemIndex = pdcb->ByteSize - (BYTE)StrToInt(CbByteSize->Items->operator [](0));
	CbStopBits->ItemIndex = pdcb->StopBits;
  	CbFlow->ItemIndex     = pdcb->fDtrControl;    //TYbCommDevice::fcNone;
  	EditAutoAns->Text     = 0;    //No Auto Answer
  	EditInBufSize->Text   = IntToStr(inSize); //8kbytes
  	EditOutBufSize->Text  = IntToStr(outSize); //8kbytes
}

void TCommDev_Setting::InitDCB(const int portidx,
        const DWORD BaudRate,       /* Baudrate at which running       */
        const BYTE Parity,          /* 0-4=None,Odd,Even,Mark,Space    */
        const BYTE ByteSize,        /* Number of bits/byte, 4-8        */
        const BYTE StopBits        /* 0,1,2 = 1, 1.5, 2               */
    )
{
    CbPort->Text = "COM" + IntToStr(portidx);
    for (int idx = 0; idx < CbSelBaud->Items->Count; idx++){
        if ((DWORD)StrToInt(CbSelBaud->Items->operator [](idx) ) == BaudRate){
                CbSelBaud->ItemIndex = idx;
                    break;
            }
    }
    CbParity->ItemIndex = Parity;
	CbByteSize->ItemIndex = ByteSize - (BYTE)StrToInt(CbByteSize->Items->operator [](0));
	CbStopBits->ItemIndex = StopBits;
}

void TCommDev_Setting::GetDCB(int& portidx,
        DWORD& BaudRate,       /* Baudrate at which running       */
        BYTE& Parity,          /* 0-4=None,Odd,Even,Mark,Space    */
        BYTE& ByteSize,        /* Number of bits/byte, 4-8        */
        BYTE& StopBits        /* 0,1,2 = 1, 1.5, 2               */)
{
    portidx = StrToInt(CbPort->Text.SubString(4,3));
    BaudRate = StrToInt(CbSelBaud->Items->operator [](CbSelBaud->ItemIndex));
    Parity = CbParity->ItemIndex;
	ByteSize = CbByteSize->ItemIndex + (BYTE)StrToInt(CbByteSize->Items->operator [](0));
	StopBits = CbStopBits->ItemIndex;
}

//用当前数据设置DCB数据
int TCommDev_Setting::SetDCB(DCB *pdcb)
{
	return SetDCB(pdcb, NULL, NULL);
}

int TCommDev_Setting::SetDCB(DCB *pdcb, DWORD* pinSize, DWORD *poutSize)
{
	pdcb->BaudRate = StrToInt(CbSelBaud->Text);
	pdcb->Parity = CbParity->ItemIndex;
	pdcb->ByteSize = CbByteSize->ItemIndex + (BYTE)StrToInt(CbByteSize->Items->operator [](0));
	pdcb->StopBits = (CbStopBits->ItemIndex);
  	pdcb->fDtrControl = CbFlow->ItemIndex;

        if (pinSize) *pinSize = StrToInt(EditInBufSize->Text);
        if (poutSize) *poutSize = StrToInt(EditOutBufSize->Text);
        
        return StrToInt(CbPort->Text.SubString(4, CbPort->Text.Length()-3));
}

int __fastcall TCommSerialPortInfo::PortListSortCompare(TStringList* lpList, int Index1, int Index2)
{
  return PortNo(lpList->Strings[Index1]) - PortNo(lpList->Strings[Index2]);
}
//---------------------------------------------------------------------------
void __fastcall TCommDev_Setting::BnDefaultClick(TObject *Sender)
{
	LoadDefaultSetting();
}

void __fastcall TCommDev_Setting::LoadDefaultSetting()
{
	CbSelBaud->ItemIndex  = 12;   //TYbCommDevice::br115200;
	CbParity->ItemIndex   = 0;    //TYbCommDevice::ptNoParity;
	CbByteSize->ItemIndex = 8 - 5;
	CbStopBits->ItemIndex = 0;    //TYbCommDevice::sbOneStopBit;
  	CbFlow->ItemIndex     = 0;    //TYbCommDevice::fcNone;
  	EditAutoAns->Text     = 0;    //No Auto Answer
  	EditInBufSize->Text   = 8192; //8kbytes
  	EditOutBufSize->Text  = 8192; //8kbytes
}
//---------------------------------------------------------------------------
void __fastcall TCommDev_Setting::BnCancelClick(TObject *Sender)
{
	FIsValid = false;
        ModalResult = mrCancel;
}
//---------------------------------------------------------------------------


bool __fastcall TCommDev_Setting::GetIsValid()
{
	return FIsValid;
}
void __fastcall TCommDev_Setting::BnOKClick(TObject *Sender)
{
	FIsValid = true;
        ModalResult = mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TCommDev_Setting::FormActivate(TObject *Sender)
{
	CbPort->SetFocus();	
}
//---------------------------------------------------------------------------


