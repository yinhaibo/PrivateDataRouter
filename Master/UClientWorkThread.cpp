//---------------------------------------------------------------------------


#pragma hdrstop

#include "UClientWorkThread.h"
#include "LogFileEx.h"
#include "Tools.h"

extern LogFileEx logger;

//---------------------------------------------------------------------------
//Client Work Thread for Server Socket Threads
//---------------------------------------------------------------------------
// ClientWorkThread Implements
//---------------------------------------------------------------------------
__fastcall ClientWorkThread::ClientWorkThread(
            TServerClientWinSocket *sock,
            AnsiString& name,
            ploop_buff_t lpbufRx, ploop_buff_t lpbufTx,
            TCriticalSection* csBuffer)
    : TServerClientThread(true, sock), FName(name)
{
    _lpbufRx = lpbufRx;
    _lpbufTx = lpbufTx;
    _csBuffer = csBuffer;

    logger.Log("ClientWorkThread " + FName + " new["
        + IntToStr(GetCurrentThreadId()) + "]");
    Resume();
}

#define CWT_SOCK_RECV_STATUS 0
#define CWT_BUFF_RECV_STATUS 1

void __fastcall ClientWorkThread::ClientExecute(void)
{
    TWinSocketStream *stream;
    int rdLen;
    int wtLen;
    int iWriteBytes = 0;
    int iRecvStatus = CWT_SOCK_RECV_STATUS;
    unsigned char ucByte;
    LogMsg("ClientWorkThread is running[" + IntToStr(GetCurrentThreadId()) + "]");
    Event(seConnect);
    stream = new TWinSocketStream(ClientSocket, 500);
    while ((!this->Terminated) && (ClientSocket->Connected))
    {
        // Data read
        switch(iRecvStatus){
        case CWT_SOCK_RECV_STATUS:
            try{
                ClientSocket->Lock();
                //LogMsg("Wait for data 10ms");
                if(stream->WaitForData(10)){
                    if(!((!this->Terminated) && (ClientSocket->Connected)))  break;
                    try{
                        rdLen = stream->Read(ucaRecvBuff, DATA_OP_BUF_LEN);
                        LogMsg("Read " + IntToStr(rdLen));
                    }catch(Exception& e){
                        ClientSocket->Close();
                        break;
                    }
                    if (rdLen == 0){
                        //Client close the connection
                        ClientSocket->Close();
                        break;
                    }
                    LogMsg("Wait for data:" + IntToStr(loopbuff_getlen(_lpbufRx, 0))
                        + "," + IntToStr(loopbuff_getlen(_lpbufTx, 0)));
                    iRecvStatus = CWT_BUFF_RECV_STATUS;
                }
            }__finally{
                ClientSocket->Unlock();
            }
            break;
        case CWT_BUFF_RECV_STATUS:
            try{
                _csBuffer->Enter();
                if (iRecvStatus == CWT_BUFF_RECV_STATUS){
                    while(!loopbuff_isfull(_lpbufRx)
                        && iWriteBytes < rdLen){
                        ucByte = ucaRecvBuff[iWriteBytes++];
                        loopbuff_push(_lpbufRx, ucByte);
                    }
                    LogMsg("Read buffer:" + IntToStr(rdLen) + "," + IntToStr(iWriteBytes));
                    if (iWriteBytes == rdLen){
                        iWriteBytes = 0;
                        iRecvStatus = CWT_SOCK_RECV_STATUS; // Temp buff data has sent
                    }
                }
            }__finally{
                _csBuffer->Leave();
            }
            break;
        default:
            assert(true);
        }
            
        // Write to stream
        int iRdPos = 0;
        int iWtPos;
        try{
            //LogMsg("Write to peer client.");
            ClientSocket->Lock();
            try{
                _csBuffer->Enter();
                while((!this->Terminated) && (ClientSocket->Connected)
                    && !loopbuff_isempty(_lpbufTx) && iRdPos < DATA_OP_BUF_LEN){
                    ucaSendBuff[iRdPos++] = loopbuff_pull(_lpbufTx) & 0xFF;
                }
            }__finally{
                _csBuffer->Leave();
            }
            iWtPos = 0;
            while(!this->Terminated && ClientSocket->Connected
                  && iRdPos > 0 && iWtPos < iRdPos){
                wtLen = stream->Write(ucaSendBuff + iWtPos, iRdPos - iWtPos);
                if (wtLen == 0){ // Has a exception while write bytes
                    //Client close the connection
                    ClientSocket->Close();
                    break;
                }
                iWtPos += wtLen;
                if (iWtPos != iRdPos){
                    LogMsg("Socket buffer full, wait for 100ms.");
                    Sleep(100);
                    // socket buffer full, need to wait
                    break;
                }
            }
        }__finally{
            ClientSocket->Unlock();
        }        
    }//~while
    if (ClientSocket->Connected) ClientSocket->Close();
    delete stream;
    LogMsg("ClientWorkThread end.["
        + IntToStr(GetCurrentThreadId()) + "]");
}
__fastcall ClientWorkThread::~ClientWorkThread()
{
    LogMsg("ClientWorkThread destory.");
}
void ClientWorkThread::LogMsg(AnsiString msg)
{
    logger.Log("[Client:" + FName + "]\t" + msg);
}
#pragma package(smart_init)
