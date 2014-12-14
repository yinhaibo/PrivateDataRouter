//---------------------------------------------------------------------------

#ifndef UWorkThreadH
#define UWorkThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "UComm.h"
//---------------------------------------------------------------------------
// Message define
#define MAX_MESSAGE_LEN 10
#define MAX_RAW_BUFFER_SIZE 1024
#pragma pack(1)
typedef struct _timestamp_t{
    unsigned short year;
    unsigned char  mon;
    unsigned char  day;
    unsigned char  hour;
    unsigned char  min;
    unsigned char  second;
    unsigned short millisec;
}timestamp_t;  //sizeof(timestamp_t) => 9
typedef struct _message_t{
    unsigned short head;            // Message head
    unsigned short len;             // Message length
    unsigned char seq;              // Sequence of message
    timestamp_t   timestamp;        // Timestamp of message
    unsigned char clen;              // Message content length
    char content[MAX_MESSAGE_LEN];  // Message content
    unsigned short    crc16;        // CRC16 of the message from seq to content
    unsigned short tail;
}message_t;
#pragma pack()

#define MESSAGE_LEN_EXCEPT_CONTENT (5+sizeof(timestamp_t)+5)
#define MESSAGE_HEAD_LEN (2)
#define MESSAGE_LEN_LEN  (2)
#define MESSAGE_CRC_LEN (2)
#define MESSAGE_TAIL_LEN (2)
#define MESSAGE_CRC_DLEN  (sizeof(message_t) - MESSAGE_CRC_LEN - MESSAGE_TAIL_LEN)
//---------------------------------------------------------------------------
//Distribution Type
typedef enum _distribution_t
{
    UNIFORM_DISTRIBUTION,
    POISSON_DISTRIBUTION,
    NO_ERROR_DISTRIBUTION,
}distribution_t;
AnsiString GetDistributionDesc(distribution_t distri);
distribution_t GetDistributionFromDesc(AnsiString desc);
//---------------------------------------------------------------------------
// Work thread parameter define
class WorkParameter
{
public:
    __property WorkMode  Mode = { read = fGetMode , write = fSetMode };
    __property AnsiString  Configure = { read = fGetConfigure , write = fSetConfigure };
    __property int  DelayFrom = { read = fGetDelayFrom , write = fSetDelayFrom };
    __property int  DelayTo = { read = fGetDelayTo , write = fSetDelayTo };
    __property int  ErrorFrom = { read = fGetErrorFrom , write = fSetErrorFrom };
    __property int  ErrorTo = { read = fGetErrorTo , write = fSetErrorTo };

    WorkParameter(){}
    WorkParameter(const WorkParameter& value);
    WorkParameter operator=(const WorkParameter& value);
    __property AnsiString RequestMsg  = { read=GetRequestMsg, write=SetRequestMsg };
    __property AnsiString ResponseMsg  = { read=GetResponseMsg, write=SetResponseMsg };
    __property AnsiString HeadHex  = { read=GetHeadHex, write=SetHeadHex };
    __property AnsiString TailHex  = { read=GetTailHex, write=SetTailHex };
private:
    WorkMode mMode;
    AnsiString mConfigure;
    int delayFrom;
    int delayTo;
    int errorFrom;
    int errorTo;
    AnsiString FRequestMsg;
    AnsiString FResponseMsg;
    AnsiString FHeadHex;
    AnsiString FTailHex;

    WorkMode   __fastcall fGetMode(void) const;
    void       __fastcall fSetMode(WorkMode val);

    AnsiString __fastcall fGetConfigure(void) const;
    void       __fastcall fSetConfigure(AnsiString val);

    int        __fastcall fGetDelayFrom(void) const;
    void       __fastcall fSetDelayFrom(int val);

    int        __fastcall fGetDelayTo(void) const;
    void       __fastcall fSetDelayTo(int val);

    int        __fastcall fGetErrorFrom(void) const;
    void       __fastcall fSetErrorFrom(int val);

    int        __fastcall fGetErrorTo(void) const;
    void       __fastcall fSetErrorTo(int val);
    void __fastcall SetRequestMsg(AnsiString value);
    AnsiString __fastcall GetRequestMsg();
    void __fastcall SetResponseMsg(AnsiString value);
    AnsiString __fastcall GetResponseMsg();
    void __fastcall SetHeadHex(AnsiString value);
    AnsiString __fastcall GetHeadHex() const;
    void __fastcall SetTailHex(AnsiString value);
    AnsiString __fastcall GetTailHex() const;
};

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
    distribution_t FErrorDistribution;
    long FSeed;

    int sendSeq;
protected:
    // Configure
    WorkParameter mParam;
    message_t mReqMsg;
    message_t mRespMsg;
    message_t mRespErrMsg; 
    bool FPeerReady;

    message_t mReceiveMsgBuf;
    message_t mSendMsgBuf;
    int receivePos;
    int sendPos;
    bool hasDataRead;
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
    virtual bool __fastcall onSendMessage(message_t& msg);
    virtual message_t* __fastcall onReceiveMessage();
    // reconnect, do nothing, subclass will override
    virtual void __fastcall onReStart(){};

    void LogMsg(AnsiString msg);

    int __fastcall getRandRange(int from , int to);

    virtual void __fastcall fillMessageStruct(message_t& msg);

    //Work Thread subclass need to implement send and receive functions.
    virtual int __fastcall sendData(unsigned char* pbuffer, int len) = 0;
    virtual int __fastcall receiveData(unsigned char* pbuffer, int len) = 0;
    
public:
    
    __fastcall WorkThread(const WorkParameter& param,
        const message_t* preqMsg,
        const message_t* prespMsg);
    __fastcall void Start();
    __fastcall void Stop();

    
    bool __fastcall getReconnect();
    void __fastcall setReconnect(bool value);
    
    // Reset work parameter
    __fastcall void resetParameter(const WorkParameter& param);
    __property TOpenChannelEvent OnOpenChannel  = { read=FOnOpenChannel, write=FOnOpenChannel };
    __property TCloseChannelEvent OnCloseChannel  = { read=FOnCloseChannel, write=FOnCloseChannel };
    __property TRxMsgEvent OnRxMsg  = { read=FOnRxMsg, write=FOnRxMsg };
    __property TTxMsgEvent OnTxMsg  = { read=FOnTxMsg, write=FOnTxMsg };
    __property TErrMsgEvent OnErrMsg  = { read=FOnErrMsg, write=FOnErrMsg };
    __property int Tag  = { read=FTag, write=FTag };
    __property bool ActiveMode  = { read=FActiveMode, write=FActiveMode };
    __property AnsiString Name  = { read=FName, write=FName };
    __property distribution_t ErrorDistribution  = { read=FErrorDistribution, write=FErrorDistribution };
    __property long Seed  = { read=FSeed, write=FSeed };
};
//---------------------------------------------------------------------------
#endif
