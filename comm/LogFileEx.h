//---------------------------------------------------------------------------
 
#ifndef LogFileExH
#define LogFileExH
#include <assert.h>
 
#include "UnitLogWriter.h"
 
//---------------------------------------------------------------------------
class LogFileEx : public LogFile
{
protected:
    char *_szPath;
    char _szLastDate[9];
    int _iType;
    void SetPath(const char *szPath);
public:
    enum LOG_TYPE{YEAR = 0, MONTH = 1, DAY = 2};
    LogFileEx(const char *szPath = ".", LOG_TYPE iType = DAY);
    ~LogFileEx();
    const char * GetPath();
    void Log(LPCVOID lpBuffer, DWORD dwLength);
    void Log(const char *szText);
    void Log(const AnsiString&szText);
private://ÆÁ±Îº¯Êý
    LogFileEx(const LogFileEx&);
    LogFileEx&operator = (const LogFileEx&);
};
#endif
