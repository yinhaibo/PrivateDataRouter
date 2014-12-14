//----------------------------------------------------------------------------
#ifndef SocketCfgH
#define SocketCfgH
//----------------------------------------------------------------------------
#include <vcl\ExtCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Controls.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\Classes.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Windows.hpp>
#include <vcl\System.hpp>
//----------------------------------------------------------------------------
class TOKRightDlg : public TForm
{
__published:
	TButton *OKBtn;
	TButton *CancelBtn;
        TEdit *edtPort;
        TLabel *lblIP;
        TEdit *edtIP;
        TLabel *Label1;
        TRadioButton *rClient;
        TRadioButton *rServer;
        TLabel *Label2;
        TComboBox *cboSockType;
        void __fastcall OKBtnClick(TObject *Sender);
        void __fastcall CancelBtnClick(TObject *Sender);
        void __fastcall rServerClick(TObject *Sender);
        void __fastcall rClientClick(TObject *Sender);
private:
public:
	virtual __fastcall TOKRightDlg(TComponent* AOwner);
        bool bValid;
};
//----------------------------------------------------------------------------
//extern PACKAGE TOKRightDlg *OKRightDlg;
//----------------------------------------------------------------------------
#endif    
