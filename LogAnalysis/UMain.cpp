//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <Classes.hpp>
#include <Dialogs.hpp>
#include <IdGlobal.hpp>
#include <stdio.h>

#include "UMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFMain *FMain;

//---------------------------------------------------------------------------
//LogTime
LogTime::LogTime()
{
    year = 0;
    month = 0;
    day = 0;
    hour = 0;
    minute = 0;
    second = 0;
    millisec = 0;
}

LogTime::LogTime(unsigned short year, unsigned short month, unsigned short day)
{
    this->year = year;
    this->month = month;
    this->day = day;
    this->hour = 0;
    this->minute = 0;
    this->second = 0;
    this->millisec = 0;
}
LogTime::LogTime(unsigned short year, unsigned short month, unsigned short day,
    unsigned short hour, unsigned short minute, unsigned short second)
{
    this->year = year;
    this->month = month;
    this->day = day;
    this->hour = hour;
    this->minute = minute;
    this->second = second;
    this->millisec = 0;
}
LogTime::LogTime(unsigned short year, unsigned short month, unsigned short day,
    unsigned short hour, unsigned short minute, unsigned short second, unsigned short millisec)
{
    this->year = year;
    this->month = month;
    this->day = day;
    this->hour = hour;
    this->minute = minute;
    this->second = second;
    this->millisec = millisec;
}
LogTime::LogTime(const LogTime& rhs)
{
    this->year = rhs.year;
    this->month = rhs.month;
    this->day = rhs.day;
    this->hour = rhs.hour;
    this->minute = rhs.minute;
    this->second = rhs.second;
    this->millisec = rhs.millisec;
}
LogTime LogTime::GetCurrentTime()
{
    SYSTEMTIME vtm;
    ::GetLocalTime(&vtm);

    return LogTime(vtm.wYear, vtm.wMonth, vtm.wDay,
        vtm.wHour, vtm.wMinute, vtm.wSecond, vtm.wMilliseconds);
}
bool __fastcall LogTime::operator <(const LogTime& rhs) const
{
    long a = year * 10000000000000 +
            month * 100000000000   +
              day * 1000000000     +
             hour * 10000000       +
           minute * 100000         +
           second * 1000           +
         millisec * 1;
    long b = rhs.year * 10000000000000 +
            rhs.month * 100000000000   +
              rhs.day * 1000000000     +
             rhs.hour * 10000000       +
           rhs.minute * 100000         +
           rhs.second * 1000           +
         rhs.millisec * 1;
    if (a < b){
        return true;
    }else{
        return false;
    }
}
bool __fastcall LogTime::operator ==(const LogTime& rhs) const
{
    if (year == rhs.year && month == rhs.month && day == rhs.day
        && hour == rhs.hour && minute == rhs.minute && second == rhs.second
        && millisec == rhs.millisec){
        return true;
    }else {
        return false;
    }
}
//---------------------------------------------------------------------------
__fastcall TFMain::TFMain(TComponent* Owner)
    : TForm(Owner)
{
    stoped = false;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::btnFile1Click(TObject *Sender)
{
    OpenDialog1->FileName = txtFileName1->Text;
    if (OpenDialog1->Execute()){
        txtFileName1->Text = OpenDialog1->FileName;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::btnFile2Click(TObject *Sender)
{
    OpenDialog1->FileName = txtFileName2->Text;
    if (OpenDialog1->Execute()){
        txtFileName2->Text = OpenDialog1->FileName;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFMain::ToolButton1Click(TObject *Sender)
{
    //Anslysis
    // Log File must record in specified time format
    // YYYY/MM/DD HH:MI:SS.SSS or
    // YYYY/MM/DD HH:MI:SS or
    // YYYY-MM-DD HH:MI:SS.SSS or
    // YYYY-MM-DD HH:MI:SS

    lstContent1->Clear();
    lstContent2->Clear();
    // Open file 1 and file 2
    // Read every line and get time
    // Compare time sequence
    // Insert list, there is no equal time, a blank line will insert
    TStringList* file1 = new TStringList();
    TStringList* file2 = new TStringList();
    int precent;
    try{
        file1->LoadFromFile(txtFileName1->Text);
        file2->LoadFromFile(txtFileName2->Text);

        LogTime logtime1, logtime2;
        bool bFinished = false;
        bool rd1, rd2;
        int idx1 = 0, idx2 = 0;
        rd1 = rd2 = true;
        ProgressBar1->Position = 0;
        ProgressBar1->Visible = true;
        AnsiString line1, line2;
        while(!bFinished && !stoped){
            precent = file1->Count>file2->Count?idx1/file1->Count:idx2/file2->Count;
            ProgressBar1->Position = precent;
            Application->ProcessMessages();
            bFinished = (idx1 >= file1->Count) ||
                        (idx2 >= file2->Count);
            if (bFinished)  break;

            if (rd1){
                while (!GetTimeFromLogLine(line1 = file1->Strings[idx1++], logtime1)){
                    if (idx1 >= file1->Count){break;}
                }
                if (idx1 >= file1->Count){ break; }
            }
            if (rd2){
                while (!GetTimeFromLogLine(line2 = file2->Strings[idx2++], logtime2)){
                    if (idx2 >= file2->Count) {break;}
                }
                if (idx2 >= file2->Count){ break; }
            }
            if (logtime1 == logtime2){
                lstContent1->Items->Add(line1);
                lstContent2->Items->Add(line2);
                rd1 = rd2 = true;
            }else if(logtime1 < logtime2){
                lstContent1->Items->Add(line1);
                lstContent2->Items->Add(" ");
                rd1 = true;
                rd2 = false;
            }else{
                lstContent1->Items->Add(" ");
                lstContent2->Items->Add(line2);
                rd1 = false;
                rd2 = true;
            }
        }
        if (rd1){
            for (int idx = idx1; idx < file1->Count; idx++){
                lstContent1->Items->Add(file1->Strings[idx]);
                lstContent2->Items->Add(" ");
            }
        }
        if (rd2){
            for (int idx = idx2; idx < file2->Count; idx++){
                lstContent1->Items->Add(" ");
                    lstContent2->Items->Add(file2->Strings[idx]);
            }
        }
    }__finally{
        delete file1;
        delete file2;
    }
    ProgressBar1->Position = 100;
    ProgressBar1->Visible = false;
}
//---------------------------------------------------------------------------
bool TFMain::GetTimeFromLogLine(AnsiString& line, LogTime& logtime)
{
    if (line.Length() < 10) return false;
    char* pline = line.c_str();
    if (isnumeric(pline[0]) && isnumeric(pline[1]) &&
        isnumeric(pline[2]) && isnumeric(pline[3]) &&
        isnumeric(pline[5]) && isnumeric(pline[6]) &&
        isnumeric(pline[8]) && isnumeric(pline[9])){
        //keep to check split by date
        if ((pline[4] == '-' && pline[7] == '-')){
            sscanf(pline, "%d-%d-%d",
                &logtime.year, &logtime.month, &logtime.day);
        }else if ((pline[4] == '/' &&  pline[7] == '/')){
            sscanf(pline, "%d/%d/%d",
                &logtime.year, &logtime.month, &logtime.day);
        }else{
            return false;
        }
    }else{
        return false;
    }
    if (line.Length() < 19) return true;
    /*if (IsNumeric(pline[11]) && IsNumeric(pline[12]) &&
        IsNumeric(pline[14]) && IsNumeric(pline[15]) &&
        IsNumeric(pline[17]) && IsNumeric(pline[18])){
        //keep to check split by date
        if ((pline[13] == ':' && pline[16] == ':')){*/
            //if (pline[19] == '.'){
                sscanf(pline+11, "%d:%d:%d.%d",
                    &logtime.hour, &logtime.minute, &logtime.second,
                    &logtime.millisec);
            //}else{
            //    sscanf(pline, "%d:%d:%d",
            //        &logtime.hour, &logtime.minute, &logtime.second);
            //}
           return true;
         /*}else{
            return false;
        }
    } */
    //return false;
}
//---------------------------------------------------------------------------
void __fastcall TFMain::lstContent1Click(TObject *Sender)
{
    lstContent2->ItemIndex = lstContent1->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::lstContent2Click(TObject *Sender)
{
    lstContent1->ItemIndex = lstContent2->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::lstContent1MeasureItem(TWinControl *Control,
      int Index, int &Height)
{
    lstContent2->TopIndex = lstContent1->TopIndex;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::lstContent1MouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    if (lstContent2->TopIndex != lstContent1->TopIndex){
        lstContent2->TopIndex = lstContent1->TopIndex;
    }
}
//---------------------------------------------------------------------------


void __fastcall TFMain::lstContent2MouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
    if (lstContent1->TopIndex != lstContent2->TopIndex){
        lstContent1->TopIndex = lstContent2->TopIndex;
    }
}
bool __fastcall TFMain::isnumeric(unsigned char c)
{
    return ('0' <= c && c <= '9');
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormCreate(TObject *Sender)
{
    ProgressBar1->Visible = false;    
}
//---------------------------------------------------------------------------


void __fastcall TFMain::FormClose(TObject *Sender, TCloseAction &Action)
{
    stoped = true;    
}
//---------------------------------------------------------------------------

