//---------------------------------------------------------------------------

#ifndef UMasterMessageHandlerH
#define UMasterMessageHandlerH

#include <ScktComp.hpp>
#include "UComm.h"
#include "UMsgQueue.h"


typedef enum _NetMasterRecvStatus{
    NET_MASTER_RECV_HEAD,
    NET_MASTER_RECV_LEN,
    NET_MASTER_RECV_DATA
}NetMasterRecvStatus;

class MasterWorkThread;
class MasterMessageHandler
{
private:
    MasterWorkThread* FMaster;

    int receivePos;
    net_msg_len_t messageLen;
    int sendPos;
    int sendMessageLen;

    unsigned char receiveBuf[NETMESSAGE_MAX_LEN];
    unsigned char sendBuf[NETMESSAGE_MAX_LEN];

    unsigned int txMsgCnt;
    unsigned int rxMsgCnt;


    NetMasterRecvStatus recvStatus;

    unsigned int lastReportTick;

    IQueue* queue;

    
    void __fastcall onSendMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
    Msg * __fastcall onReceiveMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);

    // Get data from sock
    // The function using Wait for data from socket
    // and using offset to get proper data from stream
    #define CODE_CONN_CLOSE -1
    #define CODE_THREAD_TERMINATE -2
    #define CODE_NEED_CALL_NEXT 0
    int getDataFromSock(TCustomWinSocket* client, TWinSocketStream *stream,
        unsigned char* buffer, int len, int& offset);
    int FOffset;
public:
    MasterMessageHandler(
        MasterWorkThread* master);
    void LogMsg(AnsiString msg);
        
    void __fastcall ProcessMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);
};
//---------------------------------------------------------------------------
#endif
