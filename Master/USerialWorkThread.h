//---------------------------------------------------------------------------

#ifndef USerialWorkThreadH
#define USerialWorkThreadH
//---------------------------------------------------------------------------
#include "YbCommDevice.h"
#include "UWorkThread.h"

#include <Classes.hpp>
//---------------------------------------------------------------------------
class SerialWorkThread : public WorkThread
{
private:
    TYbCommDevice* mDevice;
    RawMsg mReceiveMsgBuf;
    int receivePos;

    void initParameters();

    TYbCommDevice::TBaudRate __fastcall transBaudRate(int baud);
    TYbCommDevice::TParity __fastcall transParity(char parity);
    TYbCommDevice::TStopBits __fastcall transStopBits(float stopbits);
protected:
    virtual void __fastcall onSendMessage(RawMsg& msg);
    virtual RawMsg* __fastcall onReceiveMessage();
    virtual void __fastcall onStart();
    virtual void __fastcall onStop();
    virtual void __fastcall onParameterChange();
public:
    __fastcall SerialWorkThread(WorkParameter& param);
    virtual __fastcall ~SerialWorkThread();
};
//---------------------------------------------------------------------------
#endif
