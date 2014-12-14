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

//ͨѶ�˿�����
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

//�ӿ�
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
	commport_t m_CurCommPort; //��ǰʹ�õ�ͨѶ��
        void SetCommPort(commport_t commport);
        commport_t GetCommPort();
        bool m_Connected;
        unsigned char m_RecvBuf[RxBufMax];
        TObjectQueue *m_Recv;   //���ն���
        TObjectQueue *m_Send;   //���Ͷ��У����û���õ�����Ϊ���ϲ�ӵ����ݺ󣬾����̷��ͳ�ȥ�ˣ�
        UINT32 m_RecvLen;

	//SocketͨѶ��ʽר��
	AnsiString FIPServer;
	int FIPPort;
	int FConnectTimeout;
        TStrings *clients; // ��ǰ�����ϵ�TCP�ͻ���
        TStrings *udpclients; // ��ǰ�����ϵ�UDP�ͻ���
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
        // ���ڵ��ü����շ�����
        void (__closure*m_pfcTxRx)(UINT32 tx, UINT32 rx);

        //����д��־
        
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

        //ͨѶ����ʵ�֣��ӿڵ�ʵ�֣�
        virtual TMemoryStream* Read();
        virtual TMemoryStream* View();
        virtual int Write(TMemoryStream *);  //�������û�ã����Ժ������Socketͨ�ŵĻ����������������
        virtual int Write(UINT8 *pstream, UINT32 szLen);
        virtual int Open();
        virtual int Close();
        virtual bool Connected();

	//����
        AnsiString GetConnStatus();
        void SendToRecvBuf(UINT8 *pstream, UINT32 szLen);   //������ݵ����ջ��壨ֻ���ڽ�������ʱʹ�ã�

	//Socketר��
        void BindDisconnectEvent(void (__closure*disconnect)());   //�󶨶Ͽ�����
        void BindConnectErrEvent(void (__closure*connecterr)());   //�����Ӵ���
        void BindTextMessageEvent(int (__closure*txtmessage)(AnsiString)); // ���ı��¼���Ϣ 

	//����ר��
        void BindTxRxEvent(void (__closure*txrxevent)(UINT32 tx, UINT32 rx));// �������շ��¼�
        void ResetTxRx();
        DCB dcb;
        int portidx;
        DWORD inBuffSize;
        DWORD outBuffSize;
	TCommThread *m_CommThread;    //�����߳���ָ��
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

	//����
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