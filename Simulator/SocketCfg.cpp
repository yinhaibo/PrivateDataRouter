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

#include "SocketCfg.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
//TOKRightDlg *OKRightDlg;
//--------------------------------------------------------------------- 
__fastcall TSocketSetting::TSocketSetting(TComponent* AOwner)
	: TForm(AOwner), bValid(false)
{
}
//---------------------------------------------------------------------
void __fastcall TSocketSetting::OKBtnClick(TObject *Sender)
{
	bValid = true;
	this->Hide();	
}
//---------------------------------------------------------------------------

void __fastcall TSocketSetting::CancelBtnClick(TObject *Sender)
{
	bValid = false;
	this->Close();
}
//---------------------------------------------------------------------------

void __fastcall TSocketSetting::rServerClick(TObject *Sender)
{
        edtIP->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TSocketSetting::rClientClick(TObject *Sender)
{
       edtIP->Enabled = true;       
}
//---------------------------------------------------------------------------

