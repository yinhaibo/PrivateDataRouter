//---------------------------------------------------------------------------

#ifndef UClientWorkThreadH
#define UClientWorkThreadH

#include <ScktComp.hpp>

extern "C"{
#include "loopbuffer.h"
}
//Client Work Thread
//Just forward message to Server Work thread
#define DATA_OP_BUF_LEN 512
class ClientWorkThread : public TServerClientThread
{
    private:
        TCriticalSection* _csBuffer;
        ploop_buff_t _lpbufRx;
        ploop_buff_t _lpbufTx;

        unsigned char ucaRecvBuff[DATA_OP_BUF_LEN];
        
        void __fastcall ClientExecute(void);
        void LogMsg(AnsiString msg);
    public:
        __fastcall ClientWorkThread(TServerClientWinSocket *sock,
            ploop_buff_t lpbufRx, ploop_buff_t lpbufTx,
            TCriticalSection* csBuffer);
        __fastcall ~ClientWorkThread();
};
//---------------------------------------------------------------------------
#endif
