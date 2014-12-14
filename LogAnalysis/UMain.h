//---------------------------------------------------------------------------

#ifndef UMainH
#define UMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class LogTime
{
public:
    unsigned short year;
    unsigned short  month;
    unsigned short  day;
    unsigned short  hour;
    unsigned short  minute;
    unsigned short  second;
    unsigned short  millisec;

    LogTime();
    LogTime(unsigned short year, unsigned short month, unsigned short day);
    LogTime(unsigned short year, unsigned short month, unsigned short day,
        unsigned short hour, unsigned short minute, unsigned short second);
    LogTime(unsigned short year, unsigned short month, unsigned short day,
        unsigned short hour, unsigned short minute, unsigned short second, unsigned short millisec);
    LogTime(const LogTime& rhs);
    LogTime GetCurrentTime();
    bool __fastcall operator <(const LogTime& rhs) const;
    bool __fastcall operator ==(const LogTime& rhs) const;
};
//---------------------------------------------------------------------------
class TFMain : public TForm
{
__published:	// IDE-managed Components
    TToolBar *ToolBar1;
    TToolButton *ToolButton1;
    TImageList *ImageList1;
    TPanel *Panel1;
    TLabel *Label1;
    TEdit *txtFileName1;
    TButton *btnFile1;
    TSplitter *Splitter1;
    TPanel *Panel2;
    TLabel *Label2;
    TEdit *txtFileName2;
    TButton *btnFile2;
    TListBox *lstContent1;
    TListBox *lstContent2;
    TOpenDialog *OpenDialog1;
    TSaveDialog *SaveDialog1;
    TProgressBar *ProgressBar1;
    void __fastcall btnFile1Click(TObject *Sender);
    void __fastcall btnFile2Click(TObject *Sender);
    void __fastcall ToolButton1Click(TObject *Sender);
    void __fastcall lstContent1Click(TObject *Sender);
    void __fastcall lstContent2Click(TObject *Sender);
    void __fastcall lstContent1MeasureItem(TWinControl *Control, int Index,
          int &Height);
    void __fastcall lstContent1MouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
    void __fastcall lstContent2MouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
    bool GetTimeFromLogLine(AnsiString& line, LogTime& logtime);
    bool __fastcall isnumeric(unsigned char c);

    bool stoped ;
public:		// User declarations
    __fastcall TFMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------
#endif
