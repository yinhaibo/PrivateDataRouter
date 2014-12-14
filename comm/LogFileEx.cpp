//---------------------------------------------------------------------------
 
 
#pragma hdrstop
 
#include "LogFileEx.h"
#include <stdio.h> 
//---------------------------------------------------------------------------
 
#pragma package(smart_init)
//-------------------------------------------------------------------------
 
void LogFileEx::SetPath(const char *szPath)
{
       assert(szPath);
 
       WIN32_FIND_DATA wfd;
       char temp[MAX_PATH + 1] = {0};
 
       if(FindFirstFile(szPath, &wfd) == INVALID_HANDLE_VALUE && CreateDirectory(szPath, NULL) == 0)
       {
            strcat(strcpy(temp, szPath), " Create Fail. Exit Now! Error ID :");
            ltoa(GetLastError(), temp + strlen(temp), 10);
            MessageBox(NULL, temp, "Class LogFileEx", MB_OK);
            exit(1);
       }
       else
       {
            GetFullPathName(szPath, MAX_PATH, temp, NULL);
            _szPath = new char[strlen(temp) + 1];
            assert(_szPath);
            strcpy(_szPath, temp);
       }
}
//-------------------------------------------------------------------------
LogFileEx::LogFileEx(const char *szPath , LOG_TYPE iType)
{
   _szPath = NULL;
   SetPath(szPath);
   _iType = iType;
   memset(_szLastDate, 0, 9);
}
//-------------------------------------------------------------------------
LogFileEx::~LogFileEx()
{
   if(_szPath)
    delete []_szPath;
}
//-------------------------------------------------------------------------
 
const char * LogFileEx::GetPath()
{
   return _szPath;
}
//-------------------------------------------------------------------------
 
void LogFileEx::Log(LPCVOID lpBuffer, DWORD dwLength)
{
    assert(lpBuffer);
 
    char temp[50];
    static const char format[3][20] = {"%Y", "%Y%m", "%Y%m%d"};
 
    __try
    {
        Lock();
 
        time_t now = time(NULL);
        strftime(temp, 9, format[_iType], localtime(&now));
 
        if(strcmp(_szLastDate, temp) != 0)//Change File name
        {
            strcat(strcpy(_szFileName, _szPath), "\\");
            strcat(strcat(_szFileName, temp), ".log");
            strcpy(_szLastDate, temp);
            Close();
        }
        if(!OpenFile())
            return;
 
        WriteLog(lpBuffer, dwLength);
    }
    __finally
    {
        Unlock();
    }
}
//-------------------------------------------------------------------------
void LogFileEx::Log(const char *szText)
{
   Log(szText, strlen(szText));
}
//-------------------------------------------------------------------------
void LogFileEx::Log(const AnsiString&szText)
{
    Log(szText.c_str(),szText.Length());
}
