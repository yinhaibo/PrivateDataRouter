#include <vcl.h>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <TabNotBk.hpp>
#include <Tabs.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <Buttons.hpp>
#include <ADODB.hpp>
#include <DB.hpp>
#include <msxmldom.hpp>
#include <XMLDoc.hpp>
#include <xmldom.hpp>
#include <XMLIntf.hpp>
#include <jpeg.hpp>
#include <Registry.hpp>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#pragma hdrstop
#include "UComm.h"
#include "Tools.h"
#pragma package(smart_init)
#pragma link "YbCommDevice"
#pragma resource "*.dfm"

/*
 * 函数分类方法如下：
 * 1、构造函数和析构函数
 * 2、公有函数             ：取连接状态信息
 * 3、定时器处理函数       ：定时接收数据、查看连接是否中断
 * 4、接口实现函数         ：提供给上层，上层可以通过它来取数据、发数据
 * 5、属性实现函数         ：对通讯口的配置大多是通过这个来设置
 * 6、串口操作函数         ：串口通讯时用（它是默认的通讯方式）
 * 7、Socket操作函数       ：Socket通讯时用
 * 8、日志操作函数
 * 9、线程操作函数         ：这里的线程负责对串口监听、Socket数据的接收等
 */

//记录通讯日志
#define LOG_FILENAME        "\\Log\\sdep_comm.log"

//使用控件方式
//自己写的仍然有乱码接收问题，使用控件方式没有问题
//查看过控件源码，也是使用多线程，所以应该是可以靠的
//以前之所有使用多线程方式，是因为这个控件似乎有接收不过来的现象
#define USING_VICTOR_COMM_CONTROL

//这里这些全局数据，是不是放到通讯层做更好一些呢？
//如果因为线程的原因不能访问,那可以用友元来做啊
HANDLE  hCom;                  //串口的句柄
COMSTAT Rcs;                   //COMSTAT类型结构变量
DWORD   dwEvtMask;             //存放事件掩码组合值
OVERLAPPED  Eol;               //事件线程使用的OVERLAPPED结构Eol
OVERLAPPED  Wol;               //写操作使用的OVERLAPPED结构Wol
OVERLAPPED  Rol;               //读操作使用的OVERLAPPED结构Rol
unsigned  int   Rx_Head,Rx_Tail;
#define inc_Ptr(addr) { addr++;   if( addr >= RxBufMax )  addr=0;  }
unsigned  char  RxDataBuf[RxBufMax];    //用API函数实现串口通讯时，存放串口接收的数据

#define TXRX_CALL(t, r) do{ \
    if (m_pfcTxRx) m_pfcTxRx(t, r); \
    }while(0)
//--------------------
//构造函数和析构函数
//--------------------
__fastcall TFComm::TFComm(TComponent* Owner, commport_t cmpt)
        : TForm(Owner), portidx(1), m_Connected(false),
        m_CommThread(NULL), inBuffSize(1024), outBuffSize(1024),
        FListenMode(false)
{
	SetCommPort(cmpt);

        m_Recv = new TObjectQueue();
        m_Send = new TObjectQueue();

        clients = new TStringList();
        udpclients = new TStringList();
        m_sockProtocol = CMPT_PROTOCOL_TCP;

        // 初始化发送量与接收量字节数
        uiTx = uiRx = 0;

        m_pfcTextMessage = NULL;
        m_pfcConnectErr = NULL;
        m_pfcDisconnect = NULL;
}
__fastcall TFComm::~TFComm()
{
        tmrRecv->Enabled = false;
        CheckConnectTmr->Enabled = false;

	if (m_Connected)
        	Close();
        delete clients;
        delete udpclients;
        delete m_Recv;
}

//--------
//定时器
//--------
void __fastcall TFComm::tmrRecvTimer(TObject *Sender)   //定时收数据
{
        int rvlen;
        if (!Connected())
                return;

	switch(m_CurCommPort)
        {
        case CMPT_RS232:
                rvlen = GetComData(m_RecvBuf);
                if (rvlen > 0){
                        uiRx += rvlen;
                        TXRX_CALL(uiTx, uiRx);
                	SendToRecvBuf(m_RecvBuf, rvlen);
                }
                break;
        case CMPT_SOCKET:
                if (Protocol == CMPT_PROTOCOL_UDP){
                        // Read data from udp
                        rvlen = IdUDPClient1->ReceiveBuffer(
                                m_RecvBuf, RxBufMax, 10);
                        if (rvlen > 0){
                                uiRx += rvlen;
                                TXRX_CALL(uiTx, uiRx);
                	        SendToRecvBuf(m_RecvBuf, rvlen);
                        }
                }
                //定时从SOCKET产生数据包
                if (m_RecvLen > 0)
                {
                        SendToRecvBuf(m_RecvBuf, m_RecvLen);
                        uiRx += m_RecvLen;
                        TXRX_CALL(uiTx, uiRx);
                        m_RecvLen = 0;
                }
                break;
        }
}
//添加数据到接收缓冲
void TFComm::SendToRecvBuf(UINT8 *pstream, UINT32 szLen)
{
	TMemoryStream *pms = new TMemoryStream();
        pms->Write(pstream, szLen);
        m_Recv->Push(pms);
}

void __fastcall TFComm::CheckConnectTmrTimer(TObject *Sender)
{
        // Socket通讯时，定时查看连接是否中断
        // 仅在使用TCP客户端时采用，服务模式不需要重连
        /*static time_t lasttime = 0;
        if (m_CurCommPort ==CMPT_SOCKET && SockMode == CMPT_SOCK_CLIENT &&
                Protocol == CMPT_PROTOCOL_TCP)
        {
                if (lasttime == 0) lasttime = time(NULL);
                if (time(NULL) - lasttime > FConnectTimeout)
                {
                        m_pfcDisconnect();
                        CheckConnectTmr->Enabled = false;
                }
        } */
        
        // check upd peer timeout
        int i = 0;
        UDPClient *client;
        do{
                if (udpclients->Count == 0) break;
                client = (UDPClient *)udpclients->Objects[i];
                if (client == NULL) break;
                // clear client which have no data package
                // arrived in recent 600 seconds.
                if (client->Now() - time(NULL) > 600){
                        // delete this udp client
                        udpclients->Delete(i);
                        i = 0;
                }else{i++;}
        }while(i < udpclients->Count);
}

//----------
//公有函数
//----------
AnsiString TFComm::GetConnStatus()
{
        char buff[100];

        switch(m_CurCommPort)
        {
        case CMPT_RS232:
                switch(dcb.BaudRate)
                {
                case CBR_110:
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "110");
                        break;
                case CBR_300   ://CBR_300
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "300");
                        break;
                case CBR_600   ://CBR_600
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "600");
                        break;
                case CBR_1200  ://CBR_1200
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "1200");
                        break;
                case CBR_2400  ://CBR_2400
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "2400");
                        break;
                case CBR_4800  ://CBR_4800
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "4800");
                        break;
                case CBR_9600  ://CBR_9600
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "9600");
                        break;
                case CBR_14400 ://CBR_14400
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "14400");
                        break;
                case CBR_19200 ://CBR_19200
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "19200");
                        break;
                case CBR_38400 ://CBR_38400
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "38400");
                        break;
                case CBR_56000 ://CBR_56000
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "56000");
                        break;
                case CBR_57600 ://CBR_57600
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "57600");
                        break;
                case CBR_115200://CBR_115200
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "115200");
                        break;
                case CBR_128000://CBR_128000
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "128000");
                        break;
                case CBR_256000://CBR_256000
                        snprintf(buff, 100, "COM%d Baud:%s", portidx, "256000");
                        break;
                }
                break;
        case CMPT_SOCKET:
                if (Protocol == CMPT_PROTOCOL_TCP){
                switch(SockMode){
                case CMPT_SOCK_CLIENT:
                        snprintf(buff, 100, "TCP:%s, Port:%d", IPServer, IPPort);
                        break;
                case CMPT_SOCK_SERVER:
                        snprintf(buff, 100, "Listen on TCP port:%d", IPPort);
                        break;
                }
                }else if(Protocol == CMPT_PROTOCOL_UDP){
                switch(SockMode){
                case CMPT_SOCK_CLIENT:
                        snprintf(buff, 100, "UDP:%s, Port:%d", IPServer, IPPort);
                        break;
                case CMPT_SOCK_SERVER:
                        snprintf(buff, 100, "Listen UDP Port:%d", IPPort);
                        break;
                }
                }

                break;
        }

        AnsiString rv;
        if (m_Connected)
        {
                if (m_CurCommPort == CMPT_RS232)
                        rv = "Serial port has opened. ";
                else
                        rv = "Network port has opened. ";
                rv += buff;
        }
        else
        {
                if (m_CurCommPort == CMPT_RS232)
                        rv = "Serial port has closed.";
                else
                        rv = "Network port has closed.";
        }

        return rv;
}

//----------
//接口实现
//----------
TMemoryStream* TFComm::Read()
{
	UINT8 buf[RECV_BUFF_MAX_LEN];
        int rv;
        TMemoryStream *p;

        if (m_Recv->Count() > 0)
        {
                p = (TMemoryStream*)m_Recv->Pop();
                p->Seek(0, soFromBeginning);
                rv = p->Read(buf, RECV_BUFF_MAX_LEN);
                LogMsg("Read" + IntToStr(rv) + ": " + StreamToText(buf, rv));
                return p;
        }
        else return NULL;
}
TMemoryStream* TFComm::View()
{
	if (m_Recv->Count() > 0)
                return (TMemoryStream*)m_Recv->Peek();
        else
                return NULL;
}
int TFComm::Write(TMemoryStream *stream)
{
	//int rv;
        //stream->Seek(0, soFromBeginning);
        //rv = stream->Read(m_RecvBuf, MIN(stream->Size, RECV_BUFF_MAX_LEN));
        Write((UINT8 *)stream->Memory, (UINT32)stream->Size);

        return 0;
}
int TFComm::Write(UINT8 *pstream, UINT32 szLen)
{
        if (!Connected())
                return 0;
        if (FListenMode) return 0;
        int i;
        TCustomWinSocket * clientSocket;
        UDPClient * udpclient;
        int sendlen, leavebytes;
        unsigned char* sendptr;
	switch(m_CurCommPort)
        {
        case CMPT_RS232:
                PutComData(pstream, szLen);
                break;
        case CMPT_SOCKET:
                if (Protocol == CMPT_PROTOCOL_TCP){
                      // Using TCP protocol to communicate others
                      switch(SockMode)
                      {
                      case CMPT_SOCK_CLIENT:
                              leavebytes = szLen;
                              sendptr = pstream;
                              if (ClientSocket->Active){
                                do{
                                     sendlen= ClientSocket->Socket->SendBuf(sendptr, leavebytes);
                                     if (sendlen < 0){
                                        // 对端关闭
                                        ServerSocketClientDisconnect(this, ClientSocket->Socket);
                                        break;
                                     }
                                     leavebytes -= sendlen;
                                     sendptr += sendlen;
                                     uiTx += sendlen;
                                     TXRX_CALL(uiTx, uiRx);
                                     if (leavebytes != 0){
                                        WaitMilliseconds(10);
                                     }
                                }while(leavebytes != 0 && ClientSocket->Active);
                              }
                              break;
                      case CMPT_SOCK_SERVER:
                              // write data to every client which connected on server
                              if (ServerSocket->Active){
                                      for (i = 0; i < clients->Count; i++)
                                      {
                                              clientSocket = (TCustomWinSocket *)clients->Objects[i];
                                              leavebytes = szLen;
                                              sendptr = pstream;
                                              if (clientSocket->Connected){
                                                do{
                                                     sendlen= clientSocket->SendBuf(sendptr, leavebytes);
                                                     if (sendlen < 0){
                                                        // 对端关闭
                                                        ServerSocketClientDisconnect(this, clientSocket);
                                                        break;
                                                     }
                                                     leavebytes -= sendlen;
                                                     sendptr += sendlen;
                                                     uiTx += sendlen;
                                                     TXRX_CALL(uiTx, uiRx);
                                                     if (leavebytes != 0){
                                                        WaitMilliseconds(10);
                                                     }
                                                }while(leavebytes != 0 && clientSocket->Connected);
                                              }
                                        }
                              }
                              break;
                      }
                }else if (Protocol == CMPT_PROTOCOL_UDP){
                      // Using UDP protocol to communicate others
                      switch(SockMode)
                      {
                      case CMPT_SOCK_CLIENT:
                              if (IdUDPClient1->Active){
                                  IdUDPClient1->SendBuffer(pstream, szLen);
                                  uiTx += szLen;
                                  TXRX_CALL(uiTx, uiRx);
                              }
                              break;
                      case CMPT_SOCK_SERVER:
                              // write data to every client which connected on server
                              if (IdUDPServer1->Active){
                                      for (i = 0; i < udpclients->Count; i++)
                                      {
                                        // Get peer info
                                        udpclient = (UDPClient *)udpclients->Objects[i];
                                        // Send data to peer
                                        IdUDPServer1->SendBuffer(
                                              udpclient->Ip(),
                                              udpclient->Port(),
                                              pstream, szLen);
                                        uiTx += sendlen;
                                        TXRX_CALL(uiTx, uiRx);

                                      }
                              }
                              break;
                      }
                }
        }
        LogMsg("Write:" + IntToStr(szLen) + " " +StreamToText(pstream, szLen));
        return 0;
}
int TFComm::Open() //打开通讯层接口，允许接收发送数据
{
        switch(m_CurCommPort)
        {
        case CMPT_RS232:
                if(OpenCom())
                	m_Connected = true;
                else
                        m_Connected = false;
                break;
        case CMPT_SOCKET:
        	try
                {
                    if (Protocol == CMPT_PROTOCOL_TCP){
                        if (SockMode == CMPT_SOCK_CLIENT){
                            // Create connection to server
                            ClientSocket->Host = FIPServer;
                            ClientSocket->Port = FIPPort;
                            ClientSocket->Active = true;
                        }else{
                            // Start listening on local
                            ServerSocket->Port = FIPPort;
                            ServerSocket->Active = true;
                        }
                    }else if (Protocol == CMPT_PROTOCOL_UDP){
                    // Using Indy components access UDP protocol
                        if (SockMode == CMPT_SOCK_CLIENT){
                            // Create connection to server
                            IdUDPClient1->Host = FIPServer;
                            IdUDPClient1->Port = FIPPort;
                            IdUDPClient1->Active = true;
                        }else{
                            // Start listening on local
                            IdUDPServer1->Bindings->Items[0]->Port = FIPPort;
                            IdUDPServer1->Active = true;
                        }
                    }
                    m_Connected = true;
                    CheckConnectTmr->Enabled = true;
                }
                catch(Exception& exception)
                {
                        if (m_pfcTextMessage) m_pfcTextMessage("打开网络连接时出错." + exception.Message);
			m_Connected = false;
                }catch(...){
                }
                break;
        }

	return m_Connected;
}
int TFComm::Close()  //并闭通讯层接口，不收发数据
{
        int i;
	switch(m_CurCommPort)
        {
        case CMPT_RS232:
                if(StopCom())
                	m_Connected = false;
                break;
        case CMPT_SOCKET:
        	try
                {
                        m_Connected = false;
                        if (Protocol == CMPT_PROTOCOL_TCP){
                            switch(SockMode){
                            case CMPT_SOCK_CLIENT:
                                    ClientSocket->Active = false;
                                    break;
                            case CMPT_SOCK_SERVER:
                                    clients->Clear();
                                    ServerSocket->Active = false;
                                    break;
                            }
                        }else if (Protocol == CMPT_PROTOCOL_UDP){
                            switch(SockMode){
                            case CMPT_SOCK_CLIENT:
                                    IdUDPClient1->Active = false;
                                    break;
                            case CMPT_SOCK_SERVER:
                                    udpclients->Clear();
                                    IdUDPServer1->Active = false;
                                    break;
                            }
                        }

                        CheckConnectTmr->Enabled = false;
                }
                catch(...)
                {
                	if (m_pfcTextMessage) m_pfcTextMessage("关闭网络连接时出错.");
                }
                break; 
        }

	return !m_Connected;
}

//------
//属性
//------
bool TFComm::Connected()
{
	return m_Connected;
}
void TFComm::SetCommPort(commport_t commport)
{
        switch(commport)
        {
        case CMPT_RS232:
#ifdef	USING_VICTOR_COMM_CONTROL
		YbCommDevice1->Baud = TYbCommDevice::br115200;
        	YbCommDevice1->Parity = TYbCommDevice::ptNoParity;
	        YbCommDevice1->StopBits = TYbCommDevice::sbOneStopBit;
        	YbCommDevice1->ByteSize = 8;
#endif
		BuildCommDCB("baud=115200 parity=N data=8 stop=1", &dcb);
                break;
        case CMPT_SOCKET:
                ClientSocket->Host = FIPServer;
                ClientSocket->Port = FIPPort;
                break;
        }
        m_CurCommPort = commport;
}
commport_t TFComm::GetCommPort()
{
	return m_CurCommPort;
}
void __fastcall TFComm::SetIPServer(AnsiString value)
{
	if(FIPServer != value) {
		FIPServer = value;
	}
}
AnsiString __fastcall TFComm::GetIPServer()
{
	return FIPServer;
}

void __fastcall TFComm::SetIPPort(int value)
{
	if(FIPPort != value) {
		FIPPort = value;
	}
}
int __fastcall TFComm::GetIPPort()
{
	return FIPPort;
}
void __fastcall TFComm::SetConnectTimeout(int value)
{
	if(FConnectTimeout != value)
		FConnectTimeout = value;
}
int __fastcall TFComm::GetConnectTimeout()
{
	return FConnectTimeout;
}
void __fastcall TFComm::SetSockMode(commport_sock_t value)
{
        m_sockMode = value;
}

commport_sock_t __fastcall TFComm::GetSockMode()
{
        return m_sockMode;
}
void __fastcall TFComm::SetProtocol(commport_protocol_t value)
{
        m_sockProtocol = value;
}
commport_protocol_t __fastcall TFComm::GetProtocol()
{
        return m_sockProtocol;
}
//----------
//串口操作
//----------
bool TFComm::OpenCom()  //打开串口
{
#ifdef USING_VICTOR_COMM_CONTROL
        YbCommDevice1->PortNo = portidx;
        switch(dcb.BaudRate)
        {
        case CBR_110:
        	YbCommDevice1->Baud = TYbCommDevice::br110;
                break;
        case CBR_300:
        	YbCommDevice1->Baud = TYbCommDevice::br300;
        	break;
        case CBR_600:
        	YbCommDevice1->Baud = TYbCommDevice::br600;
        	break;
        case CBR_1200:
        	YbCommDevice1->Baud = TYbCommDevice::br1200;
        	break;
        case CBR_2400:
        	YbCommDevice1->Baud = TYbCommDevice::br2400;
        	break;
        case CBR_4800:
        	YbCommDevice1->Baud = TYbCommDevice::br4800;
        	break;
        case CBR_9600:
        	YbCommDevice1->Baud = TYbCommDevice::br9600;
        	break;
        case CBR_14400:
        	YbCommDevice1->Baud = TYbCommDevice::br14400;
        	break;
        case CBR_19200:
        	YbCommDevice1->Baud = TYbCommDevice::br19200;
        	break;
        case CBR_38400:
        	YbCommDevice1->Baud = TYbCommDevice::br38400;
        	break;
        case CBR_56000:
        	YbCommDevice1->Baud = TYbCommDevice::br56000;
        	break;
        case CBR_57600:
        	YbCommDevice1->Baud = TYbCommDevice::br57600;
        	break;
        case CBR_115200:
        	YbCommDevice1->Baud = TYbCommDevice::br115200;
        	break;
        case CBR_128000:
        	YbCommDevice1->Baud = TYbCommDevice::br128000;
        	break;
        case CBR_256000:
        	YbCommDevice1->Baud = TYbCommDevice::br256000;
        	break;
	default:
        	YbCommDevice1->Baud = TYbCommDevice::br115200;
        }
        switch(dcb.Parity)
        {
        case NOPARITY:
        	YbCommDevice1->Parity = TYbCommDevice::ptNoParity;
                break;
        case EVENPARITY:
        	YbCommDevice1->Parity = TYbCommDevice::ptEvenParity;
                break;
        case MARKPARITY:
        	YbCommDevice1->Parity = TYbCommDevice::ptMarkParity;
                break;
        case SPACEPARITY:
        	YbCommDevice1->Parity = TYbCommDevice::ptSpaceParity;
                break;
        }
        switch(dcb.StopBits)
        {
        case ONESTOPBIT: //1 stop bit
        	YbCommDevice1->StopBits = TYbCommDevice::sbOneStopBit;
                break;
        case ONE5STOPBITS:
        	YbCommDevice1->StopBits = TYbCommDevice::sbOne_5_StopBits;
        	break;
        case TWOSTOPBITS:
        	YbCommDevice1->StopBits = TYbCommDevice::sbTwoStopBit;
                break;
        }
        YbCommDevice1->FlowControl = TYbCustomCommDevice::fcNone;
        YbCommDevice1->ByteSize = dcb.ByteSize;
        //YbCommDevice1->HwOutSize = dcb.BaudRate / 10;
        YbCommDevice1->Active = true;
#else
	DCB myDCB;
	String ComName;
	ComName = "COM" + IntToStr(portidx);
        
	hCom = CreateFile( ComName.c_str(),
                      GENERIC_READ | GENERIC_WRITE,//访问模式允许读写
                      0, //此项必须是0
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//重叠工作方式
                      NULL);

   	if (hCom == INVALID_HANDLE_VALUE)
     	{
       		if (m_pfcTextMessage) m_pfcTextMessage("不能打开串口");
       		CloseHandle(hCom);
       		hCom = 0;
       		return 0;
     	}

        if (!GetCommState(hCom,&myDCB))
       		if (m_pfcTextMessage) m_pfcTextMessage("GetCommState failed");

   	if (!SetupComm(hCom,inBuffSize,outBuffSize)) //设置输入输出缓冲区大小
       		if (m_pfcTextMessage) m_pfcTextMessage("SetupComm failed");

	myDCB.BaudRate       = dcb.BaudRate;
   	myDCB.fParity        = dcb.fParity;
   	myDCB.fBinary        = dcb.fBinary;
   	myDCB.Parity         = dcb.Parity;
   	myDCB.StopBits       = dcb.StopBits;
   	myDCB.ByteSize       = dcb.ByteSize;
   	myDCB.fNull          = dcb.fNull;
   	if(!SetCommState(hCom, &myDCB)) //重新配置串口
      		if (m_pfcTextMessage) m_pfcTextMessage("SetCommState failed");

   	//设置事件掩码，EV_RXCHAR表示接收一个字符并放到缓冲区划
   	if (!SetCommMask(hCom,EV_RXCHAR | EV_TXEMPTY))
       		if (m_pfcTextMessage) m_pfcTextMessage("SetCommMask failed");

   	DWORD dwError = 0;
   	Rol.hEvent = CreateEvent(NULL,
                            true,
                            false,
                            NULL);
   	ClearCommError(hCom,&dwError,&Rcs);

   	//清空串口缓冲区,退出所有相关操作
   	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);

   	//创建线程
   	m_CommThread = new TCommThread(false);
#endif
   	return true;
}
bool TFComm::StopCom()   //关闭串口
{
#ifdef USING_VICTOR_COMM_CONTROL
	YbCommDevice1->Active = false;
#else
	if (hCom)
	{
		if (CloseHandle(hCom))
			m_CommThread->Terminate();
		else
			return false;
	}
	hCom = 0;
#endif
	return true;
}
void TFComm::PutComData( unsigned char *chStr, unsigned int StrLen)   //Put Data
{

        unsigned int leavebytes = StrLen;
        unsigned char* sendptr = chStr;
#ifdef USING_VICTOR_COMM_CONTROL
        unsigned int sendlen;
        //unsigned int cnt = 0;
        do{
	        sendlen = YbCommDevice1->Write(sendptr, leavebytes);
                //LogMsg("write bytes:" + IntToStr(sendlen));
                sendptr += sendlen;
                uiTx += sendlen;
                leavebytes -= sendlen;
                TXRX_CALL(uiTx, uiRx);
                //cnt++;
                if (leavebytes != 0){
                        WaitMilliseconds(1000000/dcb.BaudRate);
                }
        }while(leavebytes != 0 && YbCommDevice1->Active);
        //LogMsg("write times:" + IntToStr(cnt));
#else
        BOOL WriteState;
        unsigned long Written ;
        DWORD dwError;
        do{
                WriteState = WriteFile(hCom,//用CreateFile 获得的文件句柄
                          sendptr,//输出缓冲区首址
                          leavebytes,//要求输出的字节数
                          &Written,//实际输出字节数
                          &Wol);//重叠操作方式数据结构地址
                sendptr += Written;
                uiTx += Written;
                leavebytes -= Written;
                TXRX_CALL(uiTx, uiRx);
                if (leavebytes != 0){
                        WaitMilliseconds(1000000/dcb.BaudRate);
                }
       }while(leavebytes != 0 && FALSE != WriteState);
        if (WriteState && GetLastError()== ERROR_IO_PENDING )
                if (m_pfcTextMessage) m_pfcTextMessage("Write COM Error !!!");
#endif
}
unsigned int TFComm::GetComData( unsigned char *chStr)   //Get Data
{
#ifdef USING_VICTOR_COMM_CONTROL
	return YbCommDevice1->Read(chStr, RxBufMax);
#else
        unsigned int StrLen;

        StrLen = 0;
        while( Rx_Head != Rx_Tail )
        {
                *chStr = RxDataBuf[Rx_Head];
                inc_Ptr(Rx_Head);
                chStr++;
                StrLen++;
        }
        return(StrLen);
#endif
} 
void TFComm::SetDTR(bool On_Off)    //流控制函数
{
#ifdef USING_VICTOR_COMM_CONTROL
        YbCommDevice1->DTR = On_Off;
#else
	if (hCom == INVALID_HANDLE_VALUE)
                return;

	EscapeCommFunction(hCom, On_Off ? SETDTR : CLRDTR);
#endif
}
void TFComm::SetRTS(bool On_Off)    //流控制函数
{
#ifdef USING_VICTOR_COMM_CONTROL
        YbCommDevice1->RTS = On_Off;
#else
	if (hCom == INVALID_HANDLE_VALUE)
                return;
        EscapeCommFunction(hCom, On_Off ? SETRTS : CLRRTS);
#endif
}

bool TFComm::GetDTR()
{
#ifdef USING_VICTOR_COMM_CONTROL
        return YbCommDevice1->DTR;
#else

#endif
}

bool TFComm::GetRTS()
{
#ifdef USING_VICTOR_COMM_CONTROL
        return YbCommDevice1->RTS;
#else

#endif
}

bool TFComm::GetDSR()
{
#ifdef USING_VICTOR_COMM_CONTROL
        return YbCommDevice1->DSR;
#else

#endif
}

bool TFComm::GetCTS()
{
#ifdef USING_VICTOR_COMM_CONTROL
        return YbCommDevice1->CTS;
#else

#endif
}


//------------
//Socket操作
//------------
void __fastcall TFComm::ClientSocketRead(TObject *Sender,
      TCustomWinSocket *Socket)
{
	//从SOCKET读数据
        int iReaded;
        try
        {
                if (m_RecvLen >= RECV_BUFF_MAX_LEN)
                {
                	//缓冲数据没有及时取走，
                        SendToRecvBuf(m_RecvBuf, m_RecvLen);
                        m_RecvLen = 0;
                }
                iReaded = Socket->ReceiveBuf(m_RecvBuf + m_RecvLen,
                        RECV_BUFF_MAX_LEN - m_RecvLen);
                if (iReaded > 0)   //读数据
                        m_RecvLen += iReaded;
        }
        catch(...)
        {
                Beep() ;
                if (m_pfcTextMessage) m_pfcTextMessage("Read Socket ERROR!");
        }
}
void TFComm::BindDisconnectEvent(void (__closure*disconnect)())
{
	m_pfcDisconnect = disconnect;
}
void TFComm::BindConnectErrEvent(void (__closure*connecterr)())
{
	m_pfcConnectErr = connecterr;
}
void TFComm::BindTextMessageEvent(int (__closure*txtmessage)(AnsiString))
{
        m_pfcTextMessage = txtmessage;
}
void TFComm::BindTxRxEvent(void (__closure*txrxevent)(UINT32 tx, UINT32 rx))
{
        m_pfcTxRx = txrxevent;
}

void TFComm::ResetTxRx()
{
        uiTx = uiRx = 0;
        TXRX_CALL(uiTx, uiRx);
}
void __fastcall TFComm::ClientSocketConnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
	LogMsg("SOCKET CONNECT");
}
void __fastcall TFComm::ClientSocketConnecting(TObject *Sender,
      TCustomWinSocket *Socket)
{
	LogMsg("SOCKET CONNECTING");
}
void __fastcall TFComm::ClientSocketDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
        if (m_Connected) ClientSocketRead(Sender, Socket);
        if (m_RecvLen > 0)
        {
                //缓冲数据没有及时取走，
                SendToRecvBuf(m_RecvBuf, m_RecvLen);
                m_RecvLen = 0;
        }
	LogMsg("SOCKET DISCONNECT");
        if (m_Connected) m_pfcDisconnect();
}
void __fastcall TFComm::ClientSocketError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
        AnsiString msg = "SOCKET 错误，连接断开:" + IntToStr(ErrorCode);
        switch(ErrorEvent){
        case eeGeneral:
        	msg += "未知错误";
                break;
	case eeSend:
        	msg += "写数据错误";
                break;
	case eeReceive:
        	msg += "读数据错误";
                break;
	case eeConnect:
        	msg +="已经接收的请求无法完成";

                //ShowMessage(msg);
                break;
	case eeDisconnect:
        	msg += "客户端" + Socket->RemoteAddress + ":" + Socket->RemotePort + "非正常关闭.";
                
                break;
	case eeAccept:
        	msg += "接收客户连接错误";
                break;
        }
        if (m_CurCommPort == CMPT_SOCKET &&
                Protocol == CMPT_PROTOCOL_TCP){
                if (SockMode == CMPT_SOCK_SERVER){
                // 关闭该客户端的连接
                //clients->Delete(clients->IndexOfObject(Socket));
                        //ServerSocketClientDisconnect(this, Socket);
                }else{
                        if (Connected()){
                                m_pfcDisconnect();
                        }
                }
        }
        //LogMsg("Client disconnected-->" + Socket->RemoteAddress + ":" + Socket->RemotePort);
        if (m_pfcTextMessage) m_pfcTextMessage(msg);
        ErrorCode = 0;


                
}

//----------
//日志操作
//----------
void TFComm::LogMsg(AnsiString msg)
{
        FILE* fd;
        AnsiString wmsg;
        static bool reporterr = false;

        AnsiString filename = ExtractFilePath(Application->ExeName) + LOG_FILENAME;
        fd = fopen(filename.c_str(), "a+t");
        if (fd == NULL){
        	if (!reporterr)
                {
                	reporterr = true;
        		AnsiString msg = "文件不存在:";
	        	if (m_pfcTextMessage) m_pfcTextMessage(msg + LOG_FILENAME);

                }
                return;
        }
        wmsg = DateTimeToStr(Now()) + "[" + IntToStr(::GetTickCount()) + "] " + msg + "\n";
        fwrite(wmsg.c_str(), sizeof(char), StrLen(wmsg.c_str()), fd);
        fclose(fd);
}

//------
//线程
//------
__fastcall TCommThread::TCommThread(bool CreateSuspended)   //线程创建
       :TThread(CreateSuspended)
{
	FreeOnTerminate = true;
}
void __fastcall TCommThread::Execute(void)    //线程执行
{
	char ReadBuff[1];

	memset(&Rol,0,sizeof(OVERLAPPED));
	Rol.hEvent = CreateEvent(NULL,true,true,NULL);
	if (Rol.hEvent == NULL) Terminate();

	if (!SetCommMask(hCom,EV_RXCHAR|EV_TXEMPTY)) Terminate();

	while (!Terminated)
	{
		WaitForSingleObject(Rol.hEvent,INFINITE);

		bool WaitComEv;
		DWORD dwError;
		AnsiString Gotstr;

		DWORD ReadStat;
		DWORD BytesRead;

		//等待被监控事件发生
		WaitComEv = WaitCommEvent(hCom,&dwEvtMask,&Rol);

		if (WaitComEv)
			ClearCommError(hCom,&dwError,&Rcs);   //更新串口状态结构体,并清除所有串口硬件错误
		else if (!WaitComEv && (dwEvtMask & EV_RXCHAR)
                	&& GetLastError()==ERROR_IO_PENDING)
		{
			ClearCommError(hCom,&dwError,&Rcs);  //更新串口状态结构体,并清除所有串口硬件错误

			while (Rcs.cbInQue > 0)              // && dwEvtMask==EV_RXCHAR)使用后部分在win98中不行
			{
				ReadStat=ReadFile(hCom,          //用CreateFile 获得的文件句柄
					ReadBuff,      //输入缓冲区首址
                                 	1,             //设定读入字节数
                                 	&BytesRead,    //实际读入字节数
                                 	&Eol);         //重叠操作方式数据结构地址

               			if (!ReadStat && GetLastError()==ERROR_IO_PENDING )
               			{
                   			while (!GetOverlappedResult(hCom,&Eol,&BytesRead,true))
                   			{
                       				dwError=GetLastError();
                       				if (dwError==ERROR_IO_INCOMPLETE)
                         				continue;
                       				else
                         				break;
                   			}
               			}
               			if (BytesRead == 1){
                                	RxDataBuf[Rx_Tail] = ReadBuff[0];
	               			inc_Ptr(Rx_Tail);
                                }
//               temp = Rx_Tail;
//               inc_Ptr(temp);
//               if( temp != Rx_Head )  Rx_Tail = temp;

               			ClearCommError(hCom,&dwError,&Rcs);      //更新串口状态结构体,并清除所有串口硬件错误
          		}
       		}
   	}//~while (!Terminated)
}

void __fastcall TCommThread::DoTerminate(void)
{
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Client close connection
void __fastcall TFComm::ServerSocketClientDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
        int idx = clients->IndexOfObject(Socket);
        if (idx >= 0){
              clients->Delete(idx);
              //if (Socket->Connected) Socket->Close();
              AnsiString msg =  IntToStr(Socket->SocketHandle)+ "," + IntToStr(Socket->Connected) + ","+ Socket->RemoteAddress + ":" + Socket->RemotePort;
              LogMsg("Client disconnected-->" + msg);
              if (m_pfcTextMessage) m_pfcTextMessage("连接断开->" + msg);
        }else{
              if (m_pfcTextMessage) m_pfcTextMessage("连接断开X->"  + IntToStr(Socket->Connected) + ","+ IntToStr(Socket->SocketHandle)+ "," + Socket->RemoteAddress + ":" + Socket->RemotePort);
              LogMsg("No client!!!" + IntToStr(Socket->SocketHandle)+ "," + Socket->RemoteAddress + ":" + Socket->RemotePort);
              //if (Socket->Connected) Socket->Close();
        }
}
//---------------------------------------------------------------------------
// Client connect to server
// Adding to current client list
void __fastcall TFComm::ServerSocketClientConnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
        // get client socket object
        AnsiString msg = Socket->RemoteAddress + ":" + Socket->RemotePort;
        clients->AddObject(msg,
                Socket
                );
        LogMsg("Client connected-->" + Socket->RemoteAddress + ":" + Socket->RemotePort + "," + IntToStr(Socket->SocketHandle));
        if (m_pfcTextMessage) m_pfcTextMessage("新连接->" + IntToStr(Socket->SocketHandle) +","+ msg);
}
//---------------------------------------------------------------------------
// process UPD Data package arrived event 
void __fastcall TFComm::IdUDPServer1UDPRead(TObject *Sender,
      TStream *AData, TIdSocketHandle *ABinding)
{
       AnsiString clientID = ABinding->PeerIP + ":" + ABinding->PeerPort;
       int clientIdx;
       clientIdx = udpclients->IndexOf(clientID);
       if (clientIdx == -1){
           TObject* obj = new UDPClient(time(NULL),
                    ABinding->PeerIP,ABinding->PeerPort);
           udpclients->AddObject(clientID, obj);
       }else{
                ((UDPClient*)udpclients->Objects[clientIdx])->UpdateTime(time(NULL));
       }
       //get data from udp data package
       // and send to receive buff
      int iReaded;
      try
      {
          if (m_RecvLen >= RECV_BUFF_MAX_LEN)
          {
                  //缓冲数据没有及时取走，
                  SendToRecvBuf(m_RecvBuf, m_RecvLen);
                  m_RecvLen = 0;
          }
          iReaded = AData->Read(m_RecvBuf + m_RecvLen,
                  RECV_BUFF_MAX_LEN - m_RecvLen);
          if (iReaded > 0)   //读数据
                  m_RecvLen += iReaded;
      }
      catch(...)
      {
          Beep() ;
          if (m_pfcTextMessage) m_pfcTextMessage("UPD接收数据包错误");
      }
       LogMsg("Client connected-->" + ABinding->PeerIP + ":" + ABinding->PeerPort);
}
//---------------------------------------------------------------------------
void  TFComm::WaitMilliseconds(unsigned int millisec)
{
        DWORD starttick = ::GetTickCount();
        do{
                Application->ProcessMessages();
        }while(::GetTickCount() - starttick < millisec);
}



void __fastcall TFComm::SetListenMode(bool value)
{
        if(FListenMode != value) {
                FListenMode = value;
        }
}
bool __fastcall TFComm::GetListenMode()
{
        return FListenMode;
}
