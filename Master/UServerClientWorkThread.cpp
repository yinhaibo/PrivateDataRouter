//---------------------------------------------------------------------------


#pragma hdrstop

#include "UServerClientWorkThread.h"
#include "UMasterWorkThread.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

extern LogFileEx logger;

//Create a new work thread instance
//It will be create on onGetThread Event in ServerSocket Object
ServerClientWorkThread::ServerClientWorkThread(TServerClientWinSocket* ASocket
    , MasterWorkThread* master) : TServerClientThread(true, ASocket)
{

    FMsgHandler = new MasterMessageHandler(master);
    LogMsg("ServerClientWorkThread new. [" + IntToStr(GetCurrentThreadId()) + "]");
    Resume();
}

//Work thread to do business logic
void __fastcall ServerClientWorkThread::ClientExecute(void)
{
    LogMsg("ServerClientWorkThread is running. [" + IntToStr(GetCurrentThreadId()) + "]");
    Event(seConnect);


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
    LogMsg("ServerClientWorkThread is end. [" + IntToStr(GetCurrentThreadId()) + "]");
}
//---------------------------------------------------------------------------
void ServerClientWorkThread::LogMsg(AnsiString msg)
{
    logger.Log("[Master Client]\t" + msg);
}
__fastcall ServerClientWorkThread::~ServerClientWorkThread(void)
{
    delete FMsgHandler;
    LogMsg("ServerClientWorkThread exit. [" + IntToStr(GetCurrentThreadId()) + "]");
}
