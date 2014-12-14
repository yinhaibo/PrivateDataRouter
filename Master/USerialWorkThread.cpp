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

__fastcall SerialWorkThread::SerialWorkThread(
    WorkParameter& param)
    : WorkThread(param)
{
    receivePos = 0; //Rest receive position to zero
    initParameters();
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onStart()
{
    try{
        mDevice->Active = true;
        if (FOnOpenChannel != NULL){
            FOnOpenChannel(this, true);
        }
        if (OnServerOpen != NULL){
            OnServerOpen(this, true);
        }
    }catch(...){
        if (FOnOpenChannel != NULL){
            FOnOpenChannel(this, false);
        }
        if (OnServerOpen != NULL){
            OnServerOpen(this, false);
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onStop()
{
    mDevice->Active = false;
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onParameterChange()
{
    // reinit parameter from param variant
    initParameters();
}
//---------------------------------------------------------------------------
void __fastcall SerialWorkThread::onSendMessage(RawMsg& msg)
{
    if(mDevice->Active){
        mDevice->Write(msg.message, 5);
    }
}
RawMsg* __fastcall SerialWorkThread::onReceiveMessage()
{
    if(mDevice->Active){
        long rdlen = mDevice->Read(mReceiveMsgBuf.message + receivePos,
            MESSAGE_LEN - receivePos);
        receivePos += rdlen;
        if (receivePos == MESSAGE_LEN){
            receivePos = 0;
            return &mReceiveMsgBuf;
        }else return NULL;
    }else return NULL;
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
    GetSerialConfigFromStr(mParam.Configure, portidx, baud, parity, databits, stopbits);

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
