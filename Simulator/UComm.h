//---------------------------------------------------------------------------

#ifndef UCommH
#define UCommH
#include <system.hpp>

// Work mode
// Only support Serial port and tcp client mode
enum WorkMode{
    WORK_MODE_SERIAL,
    WORK_MODE_TCP_CLIENT,
};

WorkMode GetModeFromStr(AnsiString modeStr);
AnsiString GetModeStr(WorkMode mode);
void GetSerialConfigFromStr(const AnsiString& configure,
    int& portidx,
    int& baud,
    char& parity,
    int& databits,
    float& stopbits);
void GetTCPClientConfigFromStr(const AnsiString& configure,
    AnsiString& ip,
    int& port);
int TextToStream(AnsiString text, UINT8 *pStream, int szLen);
//---------------------------------------------------------------------------
#endif
