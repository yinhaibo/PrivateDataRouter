//---------------------------------------------------------------------------


#pragma hdrstop

#include "UServerClientWorkThread.h"
#include "UMasterWorkThread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

extern LogFileEx logger;

//Create a new work thread instance
//It will be create on onGetThread Event in ServerSocket Object
ServerClientWorkThread::ServerClientWorkThread(
     MasterWorkThread* master,
     TServerClientWinSocket* ASocket,
     Controller* controller) : TServerClientThread(true, ASocket)
{
    FreeOnTerminate = false;
    FMaster = master;
    FController = controller;
    LogMsg("ServerClientWorkThread new. [" + IntToStr(GetCurrentThreadId()) + "]");
    Resume();
}

//Work thread to do business logic
void __fastcall ServerClientWorkThread::ClientExecute(void)
{
    LogMsg("ServerClientWorkThread is running. [" + IntToStr(GetCurrentThreadId()) + "]");
    Event(seConnect);
    //Register channel
    FMsgHandler = new MasterMessageHandler(FMaster, this, FController);
    try{
        TWinSocketStream *pStream = new TWinSocketStream(ClientSocket, 100);
        
        while(!this->Terminated && ClientSocket->Connected){
            FMsgHandler->ProcessMessage(ClientSocket, pStream);    
        }
        delete pStream;
    }catch(...){
        ClientSocket->Close();
    }
    if (ClientSocket->Connected) ClientSocket->Close();
    delete FMsgHandler;
    LogMsg("ServerClientWorkThread is end. [" + IntToStr(GetCurrentThreadId()) + "]");
}
//---------------------------------------------------------------------------
void ServerClientWorkThread::LogMsg(AnsiString msg)
{
    logger.Log("[Master Client]\t" + msg);
}
//---------------------------------------------------------------------------
__fastcall ServerClientWorkThread::~ServerClientWorkThread(void)
{
    LogMsg("ServerClientWorkThread exit. [" + IntToStr(GetCurrentThreadId()) + "]");
}
//---------------------------------------------------------------------------
void ServerClientWorkThread::UpdateTagList(Msg* msg)
{
    if (msg == NULL || msg->msgtype != MSGTYPE_TAGLIST) return;

    memset(taglist, 0, sizeof(taglist));
    memcpy(taglist, msg->taglist, sizeof(taglist));
}

int ServerClientWorkThread::GetChannelPriority()
{
    if (FMsgHandler == NULL || FMsgHandler->getChannel() == NULL) return 0;
    return FMsgHandler->getChannel()->getPriority();
}

void ServerClientWorkThread::UdateChannelErrorMode(const master_config_t* config)
{
    if (FMsgHandler == NULL) return;
    return FMsgHandler->UdateChannelErrorMode(config);
}

