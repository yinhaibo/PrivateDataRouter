//---------------------------------------------------------------------------

#ifndef UClientWorkThreadH
#define UClientWorkThreadH

#include "UWorkThread.h"

#include <ScktComp.hpp>
//---------------------------------------------------------------------------
class ClientWorkThread : public WorkThread
{
private:
    TClientSocket* mClient;
    
    bool userOpen;
    
    
    void initParameters();

    // Client socket event
    void __fastcall onSocketConnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketDisconnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketError(System::TObject* Sender,
        TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);

    void __fastcall socketErrorProcess();

    
protected:
    virtual void __fastcall onStart();
    virtual void __fastcall onStop();
    virtual void __fastcall onParameterChange();

    virtual int __fastcall sendData(unsigned char* pbuffer, int len);
    virtual int __fastcall receiveData(unsigned char* pbuffer, int len);
public:
    /*__fastcall ClientWorkThread(const WorkParameter& param,
        const message_t* preqMsg,
        const message_t* prespMsg);*/
    __fastcall ClientWorkThread( device_config_t* pDevCfg);
};
#endif
