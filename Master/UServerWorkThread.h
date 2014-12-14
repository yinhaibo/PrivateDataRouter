//---------------------------------------------------------------------------

#ifndef UServerWorkThreadH
#define UServerWorkThreadH

#include <ScktComp.hpp>

#include "UWorkThread.h"

class ServerWorkThread : public WorkThread
{
private:
    TServerSocket* mServer;
    TCustomWinSocket* mClient;
    RawMsg mReceiveMsgBuf;
    int receivePos;
    bool hasDataRead;
    
    void initParameters();

    // Client socket event
    void __fastcall onSocketConnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketDisconnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketRead(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketError(System::TObject* Sender,
        TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void __fastcall onServerListen(TObject *Sender, TCustomWinSocket *Socket);
protected:
    // Subclass will override there method to do something
    virtual void __fastcall onStart();
    virtual void __fastcall onStop();
    // Parameter changing
    virtual void __fastcall onParameterChange();
    // makeError : the subclass will generate a error message to send
    // The delay control by this super class.
    virtual void __fastcall onSendMessage(RawMsg& msg);
    virtual RawMsg* __fastcall onReceiveMessage();

public:
    ServerWorkThread(WorkParameter& param);
    virtual __fastcall ~ServerWorkThread();    
};
//---------------------------------------------------------------------------
#endif
