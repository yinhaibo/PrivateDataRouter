//---------------------------------------------------------------------------

#ifndef UServerClientWorkThreadH
#define UServerClientWorkThreadH

#include <ScktComp.hpp>
#include "UComm.h"
#include "UMsgQueue.h"
#include "UMasterMessageHandler.h"

class MasterWorkThread;
class ServerClientWorkThread : public TServerClientThread
{
private:
    MasterMessageHandler* FMsgHandler;

    IQueue* queue;
    void __fastcall Close(TCustomWinSocket* client);

    void LogMsg(AnsiString msg);
public:
    ServerClientWorkThread(
        TServerClientWinSocket* ASocket,
        MasterWorkThread* master);
    virtual __fastcall ~ServerClientWorkThread(void);
protected:
    virtual void __fastcall ClientExecute(void);
};
//---------------------------------------------------------------------------
#endif
 