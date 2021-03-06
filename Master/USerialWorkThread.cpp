//---------------------------------------------------------------------------


#pragma hdrstop

#include "USerialWorkThread.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall SerialWorkThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall SerialWorkThread::SerialWorkThread(device_config_t* pDevCfg,
            const AnsiString& name, Controller* controller)
    : WorkThread(pDevCfg, name, controller)
{
    receivePos = 0; //Rest receive position to zero
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onInit()
{
    initParameters();
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onStart()
{
    try{
        mDevice->Active = true;
        StartOK();
        if (OnServerOpen != NULL){
            OnServerOpen(this, true);
        }
        isConnected = true;
    }catch(...){
        if (FOnOpenChannel != NULL){
            FOnOpenChannel(this, false);
        }
        if (OnServerOpen != NULL){
            OnServerOpen(this, false);
        }
        isConnected = false;
    }
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onStop()
{
    mDevice->Active = false;
    isConnected = false;
    StopOK();
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onParameterChange()
{
    // reinit parameter from param variant
    initParameters();
}
//---------------------------------------------------------------------------
// Do send message
int __fastcall SerialWorkThread::sendData(unsigned char* pbuffer, int len)
{
    if(mDevice->Active){
        try{
            int sendLen = 0;
            while(sendLen < len){
                sendLen = mDevice->Write(pbuffer + sendPos,
                    len - sendPos);
                if (sendLen > 0){
                    sendPos += sendLen;
                }
            }
            return len;
        }catch(...){
            LogMsg("Serial port error in COM" + IntToStr(mDevice->PortNo));
            //socketErrorProcess();
            return -1;
        }
    }else{
        return 0;
    }
}
//---------------------------------------------------------------------------
// Do receive message
int __fastcall SerialWorkThread::receiveData(unsigned char* pbuffer, int len)
{
    if(!mDevice->Active){
        return -1;
    }
    long rdlen = mDevice->Read(pbuffer + receivePos,
            len - receivePos);
    //LogMsg("Received :" + IntToStr(rdlen));
    if (rdlen == -1){
        // No data to read
        receivePos = 0;
        hasDataRead = false;
        return -1;
    }
    receivePos += rdlen;
    if (receivePos == len){
        receivePos = 0;
        return len;
    }else return receivePos;
}
//---------------------------------------------------------------------------
//init parameters
void SerialWorkThread::initParameters()
{
    int portidx;
    int baud;
    char parity;
    int databits;
    float stopbits;
    GetSerialConfigFromStr(mpDevCfg->configure, portidx, baud, parity, databits, stopbits);

    if (mDevice == NULL){
        // Create Serial component
        mDevice = new TYbCommDevice(NULL);
    }
    mDevice->PortNo = portidx;
    mDevice->Baud = transBaudRate(baud);
    mDevice->Parity = transParity(parity);
    mDevice->ByteSize = databits;
    mDevice->StopBits = transStopBits(stopbits);
    mDevice->UsePackage = false;
    mDevice->PackageType = cptFrameHeadTail;
}

//---------------------------------------------------------------------------
//Utils
TYbCommDevice::TBaudRate __fastcall SerialWorkThread::transBaudRate(int baud)
{
    switch(baud)
    {
    case CBR_110:
        return TYbCommDevice::br110;
    case CBR_300:
        return TYbCommDevice::br300;
    case CBR_600:
        return TYbCommDevice::br600;
    case CBR_1200:
        return TYbCommDevice::br1200;
    case CBR_2400:
        return TYbCommDevice::br2400;
    case CBR_4800:
        return TYbCommDevice::br4800;
    case CBR_9600:
        return TYbCommDevice::br9600;
    case CBR_14400:
        return TYbCommDevice::br14400;
    case CBR_19200:
        return TYbCommDevice::br19200;
    case CBR_38400:
        return TYbCommDevice::br38400;
    case CBR_56000:
        return TYbCommDevice::br56000;
    case CBR_57600:
        return TYbCommDevice::br57600;
    case CBR_115200:
        return TYbCommDevice::br115200;
    case CBR_128000:
        return TYbCommDevice::br128000;
    case CBR_256000:
        return TYbCommDevice::br256000;
    default:
        return TYbCommDevice::br115200;
    }
}

TYbCommDevice::TParity __fastcall SerialWorkThread::transParity(char parity)
{
    switch(parity){
    case 'N':
        return TYbCommDevice::ptNoParity;
    case 'O':
        return TYbCommDevice::ptOddParity;
    case 'E':
        return TYbCommDevice::ptEvenParity;
    case 'M':
        return TYbCommDevice::ptMarkParity;
    case 'S':
        return TYbCommDevice::ptSpaceParity;
    default:
        return TYbCommDevice::ptNoParity;
    }
}

TYbCommDevice::TStopBits __fastcall SerialWorkThread::transStopBits(float stopbits)
{
    if (stopbits >= 1.8f){
        return TYbCommDevice::sbTwoStopBit;
    }else if(stopbits >= 1.2f){
        return TYbCommDevice::sbOne_5_StopBits;
    }else{
        return TYbCommDevice::sbOneStopBit;
    }
}

__fastcall SerialWorkThread::~SerialWorkThread()
{
    if (mDevice != NULL){
        delete mDevice;
    }
}
