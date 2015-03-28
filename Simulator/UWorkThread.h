//---------------------------------------------------------------------------

#ifndef UWorkThreadH
#define UWorkThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "UComm.h"


//#define MESSAGE_LEN_EXCEPT_CONTENT (5+sizeof(timestamp_t)+1+2)
#define MESSAGE_HEAD_LEN (2)
#define MESSAGE_LEN_LEN  (2)
#define MESSAGE_CRC_LEN (2)

//---------------------------------------------------------------------------
// Message Status
typedef enum _message_send_status_t{
    MESSAGE_SEND_MESSAGE,
    MESSAGE_SEND_EOFMESSAGE,
}message_send_status_t;
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
typedef void __fastcall (__closure *TMsgSeqUpdateEvent)(WorkThread* Sender,
                                                   int msgseq);
typedef enum _recv_msg_status_t
{
    RECV_MSG_STATUS_HEAD,    // Receive message head
    RECV_MSG_STATUS_LEN,     // Receive message len
    RECV_MSG_STATUS_DATA,    // Receive message data after len
}recv_msg_status_t;

// Work thread define
class WorkThread : public TThread
{            
protected:
    bool mIsRunning;
    TOpenChannelEvent FOnOpenChannel;
    TCloseChannelEvent FOnCloseChannel;
    unsigned int reconnectTick;
    bool autoReconnect;
private:
    TRxMsgEvent FOnRxMsg;
    TTxMsgEvent FOnTxMsg;
    TErrMsgEvent FOnErrMsg;
    TMsgSeqUpdateEvent FOnMsgSeqUpdate;
    int FTag;
    bool FActiveMode;

    // Generate error message while send specified message count
    // errorMsgPerMsg save the count,
    // -1 is need to re-generage a new value in random way
    // 0 is no message,
    // 1 is response an error message every response.
    // 2 is respones an error message every 2 response.
    int errorMsgPerMsg;
    int respMsgCnt;
    AnsiString FName;
    long FSeed;

    unsigned int sendSeq;

    message_send_status_t msgStatus; //Message status
protected:
    // Configure
    //WorkParameter mParam;
    device_config_t* mpDevCfg;
    //message_t mReqMsg;
    message_t mMessage;
    message_t mEOFMessage; 
    bool FPeerReady;

    message_t mReceiveMsgBuf;
    message_t mSendMsgBuf;
    int receivePos;
    int sendPos;
    int mRecvLen; // Current Receive Len
    
    bool hasDataRead;    
    bool isEnableWrite;
    
    bool bMessageOK; // The status of message received

    //Send buffer and receive buffer 
    unsigned char sendRawBuff[MAX_RAW_BUFFER_SIZE];
    unsigned char recvRawBuff[MAX_RAW_BUFFER_SIZE];


    // Receive message status
    recv_msg_status_t mMsgStatus;
    
    void __fastcall Execute();
    // Subclass will override there method to do something
    virtual void __fastcall onStart() = 0;
    virtual void __fastcall onStop() = 0;
    // Parameter changing
    virtual void __fastcall onParameterChange() = 0;
    // makeError : the subclass will generate a error message to send
    // The delay control by this super class.
    virtual bool __fastcall onSendMessage(message_t& msg, int error);
    virtual message_t* __fastcall onReceiveMessage();
    // reconnect, do nothing, subclass will override
    virtual void __fastcall onReStart(){};

    void LogMsg(AnsiString msg);
    void LogMsg(message_t* pmsg, AnsiString text);

    int __fastcall getRandRange(int from , int to);

    virtual void __fastcall fillMessageStruct(message_t& msg);

    //Work Thread subclass need to implement send and receive functions.
    virtual int __fastcall sendData(unsigned char* pbuffer, int len) = 0;
    virtual int __fastcall receiveData(unsigned char* pbuffer, int len) = 0;

    void __fastcall updateUIEvent(unsigned int& txMsgCnt,
            unsigned int& rxMsgCnt,
            unsigned int& errMsgCnt,
            DWORD& lastReportTick);
public:
    
    /*__fastcall WorkThread(const WorkParameter& param,
        const message_t* preqMsg,
        const message_t* prespMsg);*/
    __fastcall WorkThread(device_config_t* pDevCfg);
    __fastcall void Start();
    __fastcall void Stop();

    
    bool __fastcall getReconnect();
    void __fastcall setReconnect(bool value);
    
    // Reset work parameter
    __property TOpenChannelEvent OnOpenChannel  = { read=FOnOpenChannel, write=FOnOpenChannel };
    __property TCloseChannelEvent OnCloseChannel  = { read=FOnCloseChannel, write=FOnCloseChannel };
    __property TRxMsgEvent OnRxMsg  = { read=FOnRxMsg, write=FOnRxMsg };
    __property TTxMsgEvent OnTxMsg  = { read=FOnTxMsg, write=FOnTxMsg };
    __property TErrMsgEvent OnErrMsg  = { read=FOnErrMsg, write=FOnErrMsg };
    __property TMsgSeqUpdateEvent OnMsgSeqUpdate = {read = FOnMsgSeqUpdate, write=FOnMsgSeqUpdate};
    __property int Tag  = { read=FTag, write=FTag };
    __property bool ActiveMode  = { read=FActiveMode, write=FActiveMode };
    __property AnsiString Name  = { read=FName, write=FName };
    __property long Seed  = { read=FSeed, write=FSeed };
};
//---------------------------------------------------------------------------
#endif
