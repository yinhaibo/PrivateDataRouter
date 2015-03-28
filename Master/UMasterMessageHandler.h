//---------------------------------------------------------------------------

#ifndef UMasterMessageHandlerH
#define UMasterMessageHandlerH

#include <ScktComp.hpp>
#include "UComm.h"
#include "UMsgQueue.h"


typedef enum _NetMasterRecvStatus{
    NET_MASTER_RECV_HEAD,
    NET_MASTER_RECV_CMD,
    NET_MASTER_RECV_LEN,
    NET_MASTER_RECV_DATA
}NetMasterRecvStatus;

class ServerClientWorkThread;
class Controller;
class Channel;
class MasterWorkThread;
class MasterMessageHandler : public IMsgPush
{
private:
    
    int receivePos;
    net_msg_len_t messageLen;
    int sendPos;
    int sendMessageLen;

    unsigned char receiveBuf[NETMESSAGE_MAX_LEN];
    unsigned char sendBuf[NETMESSAGE_MAX_LEN];

    unsigned int txMsgCnt;
    unsigned int rxMsgCnt;

    
    
    ServerClientWorkThread* FClientThread;
    Controller* FController;
    Channel* FChannel;
    MsgQueue* FQueue;
    MasterWorkThread* FMaster;
    
    NetMasterRecvStatus recvStatus;

    unsigned int lastReportTick;
    
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

    master_config_t FConfig;
    int FMsgCnt; //Message count for error mode
    int FErrorHitIdx;
    long FSeed; 

    void ApplyErrorModeOnData(unsigned char* buffer, int len);
    int getRandRange(int from , int to);
public:
    MasterMessageHandler(
        MasterWorkThread* master,
        ServerClientWorkThread* clientthread,
        Controller* controller);
    virtual ~MasterMessageHandler();
    void LogMsg(AnsiString msg);
        
    void __fastcall ProcessMessage(TCustomWinSocket* client,
        TWinSocketStream *stream);

    virtual void Push(Msg* pmsg);

    Channel* getChannel();
    void UdateChannelErrorMode(const master_config_t* config);
};
//---------------------------------------------------------------------------
#endif
