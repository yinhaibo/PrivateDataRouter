//---------------------------------------------------------------------------
 
 
#pragma hdrstop

#include "UnitLogWriter.h"
#include <stdio.h> 
//---------------------------------------------------------------------------
#define MAX_PATH_LEN 512 
#pragma package(smart_init)
LogFile::LogFile(const char *szFileName)
{
   _szFileName = NULL;
   _hFile = INVALID_HANDLE_VALUE;
   ::InitializeCriticalSection(&_csLock);
 
   SetFileName(szFileName);
}
//-------------------------------------------------------------------------
LogFile::~LogFile()
{
    ::DeleteCriticalSection(&_csLock);
    Close();
 
    if(_szFileName)
        delete []_szFileName;
}
//-------------------------------------------------------------------------
 
bool LogFile::OpenFile()
{
    if(IsOpen())
        return true;
    if(!_szFileName)
        return false;
 
    _hFile = CreateFile(
                        _szFileName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
 
    if(!IsOpen() && GetLastError() == 2)//打开不成功， 且因为文件不存在， 创建文件
        _hFile = CreateFile(
                             _szFileName,
                             GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
 
    if(IsOpen())
        SetFilePointer(_hFile, 0, NULL, FILE_END);
    return IsOpen();
}
//-------------------------------------------------------------------------
DWORD LogFile::Write(LPCVOID lpBuffer, DWORD dwLength)
{
    DWORD dwWriteLength = 0;
    if(IsOpen())
        WriteFile(_hFile, lpBuffer, dwLength, &dwWriteLength, NULL);
    return dwWriteLength;
}
//-------------------------------------------------------------------------
void LogFile::WriteLog( LPCVOID lpBuffer, DWORD dwLength)
{
    time_t now;
    static char temp[512];
    char timestr[20];
    DWORD dwWriteLength;
    SYSTEMTIME systm;
    if(IsOpen())
    {
        //time(&now);
        //strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
        GetLocalTime(&systm);
        snprintf(temp, 512, "%d/%02d/%02d %02d:%02d:%02d.%03d tick:[%10d] %.*s\r\n",
            systm.wYear, systm.wMonth, systm.wDay,
            systm.wHour, systm.wMinute, systm.wSecond, systm.wMilliseconds,
            GetTickCount(), dwLength, lpBuffer);

        WriteFile(_hFile, temp, strlen(temp), &dwWriteLength, NULL);

        //WriteFile(_hFile, lpBuffer, dwLength, &dwWriteLength, NULL);
        
        //FlushFileBuffers(_hFile);
 
    }
}
//-------------------------------------------------------------------------
 
//-------------------------------------------------------------------------
void LogFile::SetFileName(const char *szName)
{
       assert(szName);
 
       if(_szFileName)
        delete []_szFileName;
 
       Close();
 
       _szFileName = new char[MAX_PATH_LEN + 1];
       assert(_szFileName);
       strcpy(_szFileName, szName);
}
//-------------------------------------------------------------------------
void LogFile::Close()
{
       if(IsOpen())
       {
        CloseHandle(_hFile);
        _hFile = INVALID_HANDLE_VALUE;
       }
}
//-------------------------------------------------------------------------
void LogFile::Log(LPCVOID lpBuffer, DWORD dwLength)
{
       assert(lpBuffer);
       __try
       {
        Lock();
 
        if(!OpenFile())
         return;
 
        WriteLog(lpBuffer, dwLength);
       }
       __finally
       {
        Unlock();
       }
}
