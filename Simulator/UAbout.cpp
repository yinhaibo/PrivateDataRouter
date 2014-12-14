//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "UAbout.h"
//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TAboutBox *AboutBox;
//--------------------------------------------------------------------- 
__fastcall TAboutBox::TAboutBox(TComponent* AOwner)
	: TForm(AOwner)
{

	DWORD   dwHandle,InfoSize;
	InfoSize   =   GetFileVersionInfoSize(Application->ExeName.c_str(), &dwHandle);

	char   *InfoBuf   =   new   char[InfoSize];
	GetFileVersionInfo(Application->ExeName.c_str(), 0, InfoSize,InfoBuf);

	char   *pInfoVal;
	unsigned   int   dwInfoValSize;
	VerQueryValue(InfoBuf, "\\VarFileInfo\\Translation",
        	&((void   *)pInfoVal),   &dwInfoValSize);
	AnsiString   V   =   "\\StringFileInfo\\"   +
		IntToHex(*((unsigned   short   int   *)
		pInfoVal),4)   +
		IntToHex(*((unsigned
		short   int   *)   &pInfoVal[2]),4)
		+   "\\FileVersion";

		VerQueryValue(InfoBuf,   V.c_str(),
			&((void   *)pInfoVal),
			&dwInfoValSize);
	V.SetLength(dwInfoValSize-1);


	Version->Caption = "Version:" + AnsiString(pInfoVal);

        delete[]   InfoBuf;
}
//---------------------------------------------------------------------
void __fastcall TAboutBox::OKButtonClick(TObject *Sender)
{
        Close();        
}
//---------------------------------------------------------------------------

