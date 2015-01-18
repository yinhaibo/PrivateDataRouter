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
    message_t mReceiveMsgBuf;
    int receivePos;

    void initParameters();

    TYbCommDevice::TBaudRate __fastcall transBaudRate(int baud);
    TYbCommDevice::TParity __fastcall transParity(char parity);
    TYbCommDevice::TStopBits __fastcall transStopBits(float stopbits);
protected:
    virtual void __fastcall onStart();
    virtual void __fastcall onStop();
    virtual void __fastcall onParameterChange();

    virtual int __fastcall sendData(unsigned char* pbuffer, int len);
    virtual int __fastcall receiveData(unsigned char* pbuffer, int len);
public:
    /*__fastcall SerialWorkThread(const WorkParameter& param,
        const message_t* preqMsg,
        const message_t* prespMsg);*/
    __fastcall SerialWorkThread(device_config_t* pDevCfg);
};
//---------------------------------------------------------------------------
#endif
