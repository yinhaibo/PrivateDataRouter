#ifndef UCommH
#define UCommH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ScktComp.hpp>
#include <ExtCtrls.hpp>
#include <time.h>
#include "YbCommDevice.h"
#include <Sockets.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdUDPBase.hpp>
#include <IdUDPClient.hpp>
#include <IdUDPServer.hpp>

#define RxBufMax        4096
#define RECV_BUFF_MAX_LEN 4096
class TFUser;
class TCommThread;

//通讯端口设置
typedef enum _commport_t{
        CMPT_RS232,
        CMPT_SOCKET
}commport_t;

typedef enum _commport_sock_t{
        CMPT_SOCK_CLIENT,
        CMPT_SOCK_SERVER
}commport_sock_t;

typedef enum _commport_protocol_t{
        CMPT_PROTOCOL_TCP,
        CMPT_PROTOCOL_UDP
}commport_protocol_t;

//接口
class CComm{
public:
        virtual TMemoryStream* Read() = 0;
        virtual TMemoryStream* View() = 0;
        virtual int Write(TMemoryStream *) = 0;
        virtual int Write(UINT8 *pstream, UINT32 szLen) = 0;
        virtual int Open() = 0;
        virtual int Close() = 0;
        virtual bool Connected() = 0;
};

//---------------------------------------------------------------------------
class TFComm : public TForm, public CComm
{
__published:	// IDE-managed Components
        TClientSocket *ClientSocket;
        TTimer *tmrRecv;
	TTimer *CheckConnectTmr;
        TYbCommDevice *YbCommDevice1;
        TServerSocket *ServerSocket;
        TIdUDPServer *IdUDPServer1;
        TIdUDPClient *IdUDPClient1;
        void __fastcall tmrRecvTimer(TObject *Sender);
	void __fastcall ClientSocketRead(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocketConnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocketConnecting(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocketDisconnect(TObject *Sender, TCustomWinSocket *Socket);
	void __fastcall ClientSocketError(TObject *Sender,TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
	void __fastcall CheckConnectTmrTimer(TObject *Sender);
        void __fastcall ServerSocketClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ServerSocketClientConnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall IdUDPServer1UDPRead(TObject *Sender,
          TStream *AData, TIdSocketHandle *ABinding);

private:	// User declarations
	commport_t m_CurCommPort; //当前使用的通讯口
        void SetCommPort(commport_t commport);
        commport_t GetCommPort();
        bool m_Connected;
        unsigned char m_RecvBuf[RxBufMax];
        TObjectQueue *m_Recv;   //接收队列
        TObjectQueue *m_Send;   //发送队列（这个没有用到，因为从上层接到数据后，就立刻发送出去了）
        UINT32 m_RecvLen;

	//Socket通讯方式专用
	AnsiString FIPServer;
	int FIPPort;
	int FConnectTimeout;
        TStrings *clients; // 当前连接上的TCP客户端
        TStrings *udpclients; // 当前连接上的UDP客户端
        commport_sock_t m_sockMode;   //Socket mode:client or server
        commport_protocol_t m_sockProtocol;
	void __fastcall SetIPServer(AnsiString value);
	AnsiString __fastcall GetIPServer();
	void __fastcall SetIPPort(int value);
	int __fastcall GetIPPort();
        void __fastcall SetSockMode(commport_sock_t value);
	commport_sock_t __fastcall GetSockMode();
        void __fastcall SetProtocol(commport_protocol_t value);
	commport_protocol_t __fastcall GetProtocol();
	void __fastcall SetConnectTimeout(int value);
	int __fastcall GetConnectTimeout();
        void (__closure*m_pfcDisconnect)();
        void (__closure*m_pfcConnectErr)();
        int  (__closure*m_pfcTextMessage)(AnsiString);
        // 用于调用监听收发数据
        void (__closure*m_pfcTxRx)(UINT32 tx, UINT32 rx);

        //用于写日志
        
        UINT8 m_LogBuff[RECV_BUFF_MAX_LEN];
        void LogMsg(AnsiString msg);

        //
        void WaitMilliseconds(unsigned int millisec);

        UINT32 uiTx, uiRx;
        bool FListenMode;
        void __fastcall SetListenMode(bool value);
        bool __fastcall GetListenMode();
public:		// User declarations
        __fastcall TFComm(TComponent* Owner, commport_t);
	inline __fastcall virtual ~TFComm();

        //通讯口类实现（接口的实现）
        virtual TMemoryStream* Read();
        virtual TMemoryStream* View();
        virtual int Write(TMemoryStream *);  //这个现在没用，但以后如果用Socket通信的话，可以在这上面改
        virtual int Write(UINT8 *pstream, UINT32 szLen);
        virtual int Open();
        virtual int Close();
        virtual bool Connected();

	//公用
        AnsiString GetConnStatus();
        void SendToRecvBuf(UINT8 *pstream, UINT32 szLen);   //添加数据到接收缓冲（只是在接收数据时使用）

	//Socket专用
        void BindDisconnectEvent(void (__closure*disconnect)());   //绑定断开连接
        void BindConnectErrEvent(void (__closure*connecterr)());   //绑定连接错误
        void BindTextMessageEvent(int (__closure*txtmessage)(AnsiString)); // 绑定文本事件消息 

	//串口专用
        void BindTxRxEvent(void (__closure*txrxevent)(UINT32 tx, UINT32 rx));// 绑定数据收发事件
        void ResetTxRx();
        DCB dcb;
        int portidx;
        DWORD inBuffSize;
        DWORD outBuffSize;
	TCommThread *m_CommThread;    //串口线程类指针
        bool OpenCom();
	bool StopCom();
        // ---Flow Control---
	void SetDTR(bool On_Off);
	void SetRTS(bool On_Off);
        bool GetDTR();
        bool GetRTS();
        bool GetDSR();
        bool GetCTS();
        
	void PutComData( unsigned char *chStr, unsigned int StrLen);
        unsigned int GetComData( unsigned char *chStr);

	//属性
	__property commport_t CommPort = {read=GetCommPort, write=SetCommPort };
	__property AnsiString IPServer  = { read=GetIPServer, write=SetIPServer };
	__property int IPPort  = { read=GetIPPort, write=SetIPPort };
        __property commport_sock_t SockMode = {read=GetSockMode, write=SetSockMode};
        __property commport_protocol_t Protocol = {read = GetProtocol, write=SetProtocol};
	__property int ConnectTimeout  = { read=GetConnectTimeout, write=SetConnectTimeout, default=180 };
        __property bool ListenMode  = { read=GetListenMode, write=SetListenMode };
};

//-------------------------------------------------
// Comm port access class using muti-thread
//-------------------------------------------------
class TCommThread : public TThread
{
public:
	__fastcall TCommThread(bool);
        void __fastcall Execute(void);
        void __fastcall DoTerminate(void);
};

class UDPClient : public TObject
{
private:
        time_t mNow;
        AnsiString mIp;
        int mPort;
public:
        UDPClient(time_t tnow, AnsiString ip, int port){
                mNow = tnow;
                mIp = ip;
                mPort = port;
        }
        AnsiString Ip(){return mIp;}
        time_t Now(){return mNow;}
        void UpdateTime(time_t tnow){mNow = tnow;}       
        int Port(){return mPort;}
};
#endif