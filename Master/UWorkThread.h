//---------------------------------------------------------------------------

#ifndef UWorkThreadH
#define UWorkThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "UComm.h"
#include "UMsgQueue.h"
#include "UMsg.h"

//---------------------------------------------------------------------------
// Work thread parameter define
class WorkParameter
{
public:
    __property WorkMode  Mode = { read = fGetMode , write = fSetMode };
    __property AnsiString  Configure = { read = fGetConfigure , write = fSetConfigure };
    WorkParameter(){}
    WorkParameter(WorkParameter& value);
    WorkParameter operator=(WorkParameter& value);
    __property AnsiString Source  = { read=GetSource, write=SetSource };
    __property AnsiString Destination  = { read=GetDestination, write=SetDestination };
    __property IMsgPush* MasterQueue  = { read=GetMasterQueue, write=SetMasterQueue };
private:
    WorkMode mMode;
    AnsiString mConfigure;
    AnsiString FSource;
    AnsiString FDestination;
    IMsgPush* FMasterQueue;

    WorkMode   __fastcall fGetMode(void) const;
    void       __fastcall fSetMode(WorkMode val);

    AnsiString __fastcall fGetConfigure(void) const;
    void       __fastcall fSetConfigure(AnsiString val);
    void __fastcall SetSource(AnsiString value);
    AnsiString __fastcall GetSource() const;
    void __fastcall SetDestination(AnsiString value);
    AnsiString __fastcall GetDestination() const;
    void __fastcall SetMasterQueue(IMsgPush* value);
    IMsgPush* __fastcall GetMasterQueue();
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


// Work thread define
class WorkThread : public TThread, IRawMsgPush
{            
protected:
    bool mIsRunning;
    TOpenChannelEvent FOnServerOpen;
    TOpenChannelEvent FOnOpenChannel;
    TCloseChannelEvent FOnCloseChannel;
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

    //Raw message queue
    RawMsgQueue* mRawMsgQueue;
    RawMsg* mSendMsg;
    AnsiString FName; //The message will be sent
protected:
    // Configure
    WorkParameter mParam;
    
    void __fastcall Execute();
    // Subclass will override there method to do something
    virtual void __fastcall onStart() = 0;
    virtual void __fastcall onStop() = 0;
    // Parameter changing
    virtual void __fastcall onParameterChange() = 0;
    // makeError : the subclass will generate a error message to send
    // The delay control by this super class.
    virtual void __fastcall onSendMessage(RawMsg& msg) = 0;
    virtual RawMsg* __fastcall onReceiveMessage() = 0;

    void LogMsg(AnsiString msg);
public:
    __fastcall WorkThread(WorkParameter& param);
    __fastcall virtual ~WorkThread();
    __fastcall void Start();
    __fastcall void Stop();
    // Reset work parameter
    __fastcall void resetParameter(WorkParameter& param);
    __property TOpenChannelEvent OnServerOpen  = { read=FOnServerOpen, write=FOnServerOpen };
    __property TOpenChannelEvent OnOpenChannel  = { read=FOnOpenChannel, write=FOnOpenChannel };
    __property TCloseChannelEvent OnCloseChannel  = { read=FOnCloseChannel, write=FOnCloseChannel };
    __property TRxMsgEvent OnRxMsg  = { read=FOnRxMsg, write=FOnRxMsg };
    __property TTxMsgEvent OnTxMsg  = { read=FOnTxMsg, write=FOnTxMsg };
    __property TErrMsgEvent OnErrMsg  = { read=FOnErrMsg, write=FOnErrMsg };
    __property int Tag  = { read=FTag, write=FTag };

    //IRawMsgPush
    virtual void Push(RawMsg* pmsg);
    __property AnsiString Name  = { read=FName, write=FName };
};
//---------------------------------------------------------------------------
#endif
