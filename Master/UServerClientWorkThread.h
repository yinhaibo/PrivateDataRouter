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
    Controller* FController;
    MasterWorkThread* FMaster;
    // For tag list
    char taglist[MAX_ALIAS_CNT][ALIAS_LEN+1]; // Client Alias list

    void __fastcall Close(TCustomWinSocket* client);

    void LogMsg(AnsiString msg);
public:
    ServerClientWorkThread(
        MasterWorkThread* master,
        TServerClientWinSocket* ASocket,
        Controller* controller);
    virtual __fastcall ~ServerClientWorkThread(void);


    void UpdateTagList(Msg* msg);
#ifdef ENABLE_PRIORITY
    int GetChannelPriority();
#endif
    void UdateChannelErrorMode(const master_config_t* config);

    //virtual void Push(Msg* pmsg);
protected:
    virtual void __fastcall ClientExecute(void);
};
//---------------------------------------------------------------------------
#endif
 