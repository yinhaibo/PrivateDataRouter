//---------------------------------------------------------------------------
#ifndef UnitLogWriterH
#define UnitLogWriterH
#include <vcl.h>
#include <time.h>
#include <assert.h>
//---------------------------------------------------------------------------
class LogFile
{
protected:
    CRITICAL_SECTION _csLock;
    char * _szFileName;
    HANDLE _hFile;
    bool OpenFile();//打开文件， 指针到文件尾
    DWORD Write(LPCVOID lpBuffer, DWORD dwLength);
    virtual void WriteLog( LPCVOID lpBuffer, DWORD dwLength);//写日志, 可以扩展修改
    void Lock()   { ::EnterCriticalSection(&_csLock); }
    void Unlock() { ::LeaveCriticalSection(&_csLock); }
public:
    LogFile(const char *szFileName = "Log.log");//设定日志文件名
    virtual ~LogFile();
    const char * GetFileName()
    {
       return _szFileName;
    }
 
    void SetFileName(const char *szName);//修改文件名， 同时关闭上一个日志文件
    bool IsOpen()
    {
       return _hFile != INVALID_HANDLE_VALUE;
    }
 
    void Close();
 
    void Log(LPCVOID lpBuffer, DWORD dwLength);//追加日志内容
 
    void Log(const char *szText)
    {
       Log(szText, strlen(szText));
    }
private://屏蔽函数
    LogFile(const LogFile&);
    LogFile&operator = (const LogFile&);
};
#endif
