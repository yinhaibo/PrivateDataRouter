//---------------------------------------------------------------------------

#ifndef UMasterMessageHandlerH
#define UMasterMessageHandlerH

#include <ScktComp.hpp>
#include "UComm.h"
#include "UMsgQueue.h"

class MasterWorkThread;
class MasterMessageHandler
{
private:
    MasterWorkThread* FMaster;

    int receivePos;
    net_msg_len_t messageLen;
    int sendPos;
    int sendMessageLen;

    unsigned char receiveBuf[MESSAGE_BUF_LEN];
    unsigned char sendBuf[MESSAGE_BUF_LEN];

    unsigned int txMsgCnt;
    unsigned int rxMsgCnt;
    int recvLen;

    unsigned int lastReportTick;

    IQueue* queue;

    
    void __fastcall onSendMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
    Msg * __fastcall onReceiveMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
public:
    MasterMessageHandler(
        MasterWorkThread* master);
        
    void __fastcall ProcessMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
};
//---------------------------------------------------------------------------
#endif
