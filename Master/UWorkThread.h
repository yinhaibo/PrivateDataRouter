//---------------------------------------------------------------------------

#ifndef UWorkThreadH
#define UWorkThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "UComm.h"
#include "UMsgQueue.h"
#include "UController.h"


//---------------------------------------------------------------------------
class WorkThreadMessage{

private:
    AnsiString FErrorMsg;
    int FRxMsgCnt;
    int FTxMsgCnt;
    int FErrMsgCnt;
    void __fastcall SetErrorMsg(AnsiString value);
    AnsiString __fastcall GetErrorMsg();
    void __fastcall SetRxMsgCnt(int value);
    int __fastcall GetRxMsgCnt();
    void __fastcall SetTxMsgCnt(int value);
    int __fastcall GetTxMsgCnt();
    void __fastcall SetErrMsgCnt(int value);
    int __fastcall GetErrMsgCnt();
public:
    __property AnsiString ErrorMsg  = { read=GetErrorMsg, write=SetErrorMsg };
    __property int RxMsgCnt  = { read=GetRxMsgCnt, write=SetRxMsgCnt };
    __property int TxMsgCnt  = { read=GetTxMsgCnt, write=SetTxMsgCnt };
    __property int ErrMsgCnt  = { read=GetErrMsgCnt, write=SetErrMsgCnt };
};

class WorkThread;
typedef void __fastcall (__closure *TOpenChannelEvent)(WorkThread* Sender,
                                                   bool opened);
typedef void __fastcall (__closure *TCloseChannelEvent)(WorkThread* Sender,
                                                   bool closed);
typedef void __fastcall (__closure *TRxMsgEvent)(WorkThread* Sender,
                                                   int msgcnt);
typedef void __fastcall (__closure *TTxMsgEvent)(WorkThread* Sender,
                                                   int msgcnt);
typedef void __fastcall (__closure *TErrMsgEvent)(WorkThread* Sender,
                                                   int msgcnt);


typedef enum _work_status_t{
    WORK_STATUS_WAIT,           // Wait to connect
    WORK_STATUS_DELAYCONNECT,   //
    WORK_STATUS_CONNECT,        // Wait to connected
    WORK_STATUS_LISTEN,         // Wait client to connect in server mode only.
    WORK_STATUS_WORKING,        // Wait receive and send message, go to connect while error occur.
    WORK_STATUS_CLOSE_WORKING,  // Wait process all buffer data
    WORK_STATUS_STOP            // Stoping and goto wait
}work_status_t;

typedef enum _recv_msg_status_t
{
    RECV_MSG_STATUS_HEAD,    // Receive message head
    RECV_MSG_STATUS_LEN,     // Receive message len
    RECV_MSG_STATUS_DATA,    // Receive message data after len
}recv_msg_status_t;

#define MESSAGE_HEAD_LEN (2)
#define MESSAGE_LEN_LEN  (2)
#define MESSAGE_CRC_LEN (2)

// Work thread define
class WorkThread : public TThread, IMsgPush
{            
protected:
    TOpenChannelEvent FOnServerOpen;
    TOpenChannelEvent FOnOpenChannel;
    TCloseChannelEvent FOnCloseChannel;
private:
    TRxMsgEvent FOnRxMsg;
    TTxMsgEvent FOnTxMsg;
    TErrMsgEvent FOnErrMsg;
    int FTag;
    bool FActiveMode;



    DWORD lastReportTick;
    unsigned int txMsgCnt;
    unsigned int rxMsgCnt;
    unsigned int errMsgCnt;

    // Generate error message while send specified message count
    // errorMsgPerMsg save the count,
    // -1 is need to re-generage a new value in random way
    // 0 is no message,
    // 1 is response an error message every response.
    // 2 is respones an error message every 2 response.
    int errorMsgPerMsg;
    int respMsgCnt;

    //Raw message queue
    RawMsgQueue* mRawMsgQueue;


    void processMessage();
protected:
    // Configure
    const device_config_t* mpDevCfg;
    long FSeed;
    
    AnsiString FName; //The message will be sent
    Controller* FController;
    Channel* FChannel;
    // Current message sequence which received from simulator
    unsigned int FCurrentMsgSeq;
    unsigned int FPrevMsgSeq;
    Msg* FPrevMsg; // Old message which has sent, keep for resent by master
    bool FLocalMessage;
    
    work_status_t FStatus; // Current work status
                           // Wait, Connect, working, stop
    bool isConnected; //Device is or not connected.
    //message_t mReqMsg;
    message_t mMessage;
    message_t mEOFMessage;

    //message_t mReceiveMsgBuf;
    //message_t mSendMsgBuf;

    int receivePos;
    int sendPos;
    int mRecvLen; // Current Receive Len
    bool hasDataRead;
    bool bMessageOK; // The status of message received
    
    //Send buffer and receive buffer 
    unsigned char sendRawBuff[MAX_RAW_BUFFER_SIZE];
    unsigned char recvRawBuff[MAX_RAW_BUFFER_SIZE];

    void __fastcall WorkThread::updateUIEvent(
            unsigned int& txMsgCnt,
            unsigned int& rxMsgCnt,
            unsigned int& errMsgCnt,
            DWORD& lastReportTick);
    
    // Receive message status
    recv_msg_status_t mMsgStatus;
                           
    void __fastcall Execute();
    // Subclass will override there method to do something
    virtual void __fastcall onInit() = 0;
    virtual void __fastcall onStart() = 0;
    virtual void __fastcall onStop() = 0;
    void __fastcall StartOK();
    void __fastcall StopOK();
    // Parameter changing
    virtual void __fastcall onParameterChange() = 0;

    // makeError : the subclass will generate a error message to send
    // The delay control by this super class.
    virtual bool __fastcall onSendMessage(RawMsg& msg, int error);
    virtual RawMsg* __fastcall onReceiveMessage(int error);
    
    //Work Thread subclass need to implement send and receive functions.
    virtual int __fastcall sendData(unsigned char* pbuffer, int len) = 0;
    virtual int __fastcall receiveData(unsigned char* pbuffer, int len) = 0;

    void LogMsg(AnsiString msg);
    void LogMsg(RawMsg& msg, AnsiString text);

    int __fastcall getRandRange(int from , int to);
public:
    __fastcall WorkThread(const device_config_t* pDevCfg,
            const AnsiString& name,
            Controller* controller);
    __fastcall virtual ~WorkThread();
    __fastcall void Start();
    __fastcall void Stop();
    // Reset work parameter
    __fastcall void resetParameter(device_config_t* param);
    __property TOpenChannelEvent OnServerOpen  = { read=FOnServerOpen, write=FOnServerOpen };
    __property TOpenChannelEvent OnOpenChannel  = { read=FOnOpenChannel, write=FOnOpenChannel };
    __property TCloseChannelEvent OnCloseChannel  = { read=FOnCloseChannel, write=FOnCloseChannel };
    __property TRxMsgEvent OnRxMsg  = { read=FOnRxMsg, write=FOnRxMsg };
    __property TTxMsgEvent OnTxMsg  = { read=FOnTxMsg, write=FOnTxMsg };
    __property TErrMsgEvent OnErrMsg  = { read=FOnErrMsg, write=FOnErrMsg };
    __property int Tag  = { read=FTag, write=FTag };

    //IMsgPush
    virtual void Push(Msg* pmsg);
    __property AnsiString Name  = { read=FName, write=FName };
    __property long Seed  = { read=FSeed, write=FSeed };
};
//---------------------------------------------------------------------------
#endif
