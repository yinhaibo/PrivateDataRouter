//---------------------------------------------------------------------------

#ifndef UServerWorkThreadH
#define UServerWorkThreadH

#include <ScktComp.hpp>

#include "UWorkThread.h"
#include "UClientWorkThread.h"

extern "C"{
#include "loopbuffer.h"
}

#define LOOPBUFF_MAX_LEN 1024

class ServerWorkThread : public WorkThread
{
private:
    TServerSocket* mServer;
    //TCustomWinSocket* mClient;

    TCriticalSection* csBuffer; // Lock of buffer
    loop_buff_t lpbufRx;
    loop_buff_t lpbufTx;
    unsigned char ucBufRx[LOOPBUFF_MAX_LEN];
    unsigned char ucBufTx[LOOPBUFF_MAX_LEN];

    //Client information
    ClientWorkThread * currSocketThread;
    void CloseClientConnection();
    
    void initParameters();

    // Client socket event
    void __fastcall onSocketConnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketDisconnect(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketRead(System::TObject* Sender, TCustomWinSocket* Socket);
    void __fastcall onSocketError(System::TObject* Sender,
        TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void __fastcall onServerListen(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall onServerAccept(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall onGetThread(TObject *Sender,
        TServerClientWinSocket *ClientSocket,
        TServerClientThread *&SocketThread);
protected:
    // Subclass will override there method to do something
    virtual void __fastcall onInit();
    virtual void __fastcall onStart();
    virtual void __fastcall onStop();
    // Parameter changing
    virtual void __fastcall onParameterChange();
    // makeError : the subclass will generate a error message to send
    // The delay control by this super class.
    virtual int __fastcall sendData(unsigned char* pbuffer, int len);
    virtual int __fastcall receiveData(unsigned char* pbuffer, int len);

public:
    ServerWorkThread(const device_config_t* pDevCfg,
            IQueue* masterQueue, const AnsiString& name);
    virtual __fastcall ~ServerWorkThread();    
};


//---------------------------------------------------------------------------
#endif
