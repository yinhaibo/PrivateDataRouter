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
    bool OpenFile();//���ļ��� ָ�뵽�ļ�β
    DWORD Write(LPCVOID lpBuffer, DWORD dwLength);
    virtual void WriteLog( LPCVOID lpBuffer, DWORD dwLength);//д��־, ������չ�޸�
    void Lock()   { ::EnterCriticalSection(&_csLock); }
    void Unlock() { ::LeaveCriticalSection(&_csLock); }
public:
    LogFile(const char *szFileName = "Log.log");//�趨��־�ļ���
    virtual ~LogFile();
    const char * GetFileName()
    {
       return _szFileName;
    }
 
    void SetFileName(const char *szName);//�޸��ļ����� ͬʱ�ر���һ����־�ļ�
    bool IsOpen()
    {
       return _hFile != INVALID_HANDLE_VALUE;
    }
 
    void Close();
 
    void Log(LPCVOID lpBuffer, DWORD dwLength);//׷����־����
 
    void Log(const char *szText)
    {
       Log(szText, strlen(szText));
    }
private://���κ���
    LogFile(const LogFile&);
    LogFile&operator = (const LogFile&);
};
#endif
