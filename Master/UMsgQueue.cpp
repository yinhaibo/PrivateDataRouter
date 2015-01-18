//---------------------------------------------------------------------------


#pragma hdrstop

#include "UMsgQueue.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)



///////////////////////////////////////////////////
// MsgQueue
///////////////////////////////////////////////////
MsgQueue::MsgQueue()
{
    mQueue = new TThreadList();
}

MsgQueue::~MsgQueue()
{
    Clear();
    delete mQueue;
}

void MsgQueue::Push(Msg* pmsg)
{
    mQueue->Add(pmsg);
}

Msg* MsgQueue::Pop()
{
    if (!Empty()){
        try{
            TList* lst = mQueue->LockList();
            Msg *pmsg = (Msg*)lst->First();
            lst->Delete(0);
            if (pmsg != NULL){
                return pmsg;
            }
        }__finally{
            mQueue->UnlockList();
        }
    }
    return NULL;
}
void MsgQueue::Clear()
{
    try{
        TList *list = mQueue->LockList();
        for (int i = 0; i < list->Count; i++)
        {
            delete (Msg *)list->Items[i];
        }
        list->Clear();
    }__finally{
        mQueue->UnlockList();
    }
}
bool MsgQueue::Empty() const
{
    bool bEmpty = true;
    try{
        TList *list = mQueue->LockList();
        bEmpty = (0 == list->Count);
    }__finally{
        mQueue->UnlockList();
    }
    return bEmpty;
}

int  MsgQueue::Count() const
{
    int count = 0;
    try{
        TList *list = mQueue->LockList();
        count = list->Count;
    }__finally{
        mQueue->UnlockList();
    }
    return count;
}
///////////////////////////////////////////////////
// RawMsgQueue
///////////////////////////////////////////////////
RawMsgQueue::RawMsgQueue()
{
    mQueue = new TThreadList();
}

RawMsgQueue::~RawMsgQueue()
{
    Clear();
    delete mQueue;
}

void RawMsgQueue::Push(RawMsg* pmsg)
{
    mQueue->Add(pmsg);
}

RawMsg* RawMsgQueue::Pop()
{
    if (!Empty()){
        try{
            TList* lst = mQueue->LockList();
            RawMsg *pmsg = (RawMsg*)lst->First();
            lst->Delete(0);
            if (pmsg != NULL){
                return pmsg;
            }
        }__finally{
            mQueue->UnlockList();
        }
    }
    return NULL;
}

void RawMsgQueue::Clear()
{
    try{
        TList *list = mQueue->LockList();
        for (int i = 0; i < list->Count; i++)
        {
            delete (Msg *)list->Items[i];
        }
        list->Clear();
    }__finally{
        mQueue->UnlockList();
    }
}

bool RawMsgQueue::Empty() const
{
    bool bEmpty = true;
    try{
        TList *list = mQueue->LockList();
        bEmpty = (0 == list->Count);
    }__finally{
        mQueue->UnlockList();
    }
    return bEmpty;
}

int  RawMsgQueue::Count() const
{
    int count = 0;
    try{
        TList *list = mQueue->LockList();
        count = list->Count;
    }__finally{
        mQueue->UnlockList();
    }
    return count;
}


int BuildNetMessage(
        const Msg* pmsg,
        unsigned char* netmsgbuf, unsigned int buflen)
{
    /** net message length: NETMESSAGE_HEADLEN bytes + from alias length(include end char)
        + to alias bytes(include end char) + message
    **/
    if (pmsg == NULL) return -1;
    net_msg_len_t totallen = NETMESSAGE_HEADLEN + 1 + strlen(pmsg->from) + 1 + strlen(pmsg->to)
        + MESSAGELEN(&pmsg->rawmsg);
    if (buflen < (unsigned int)totallen){
        return -1;
    }else{
        unsigned char* pwrite = netmsgbuf;
        unsigned char alen;
        memcpy(pwrite, &totallen, NETMESSAGE_HEADLEN); pwrite += NETMESSAGE_HEADLEN;
        alen = strlen(pmsg->from)+1;
        memcpy(pwrite, pmsg->from, alen); pwrite += alen;
        alen = strlen(pmsg->to)+1;
        memcpy(pwrite, pmsg->to, alen); pwrite += alen;
        memcpy(pwrite, &pmsg->rawmsg, sizeof(RawMsg));

        int headpartlen = offsetof(message_t, content);
        memcpy(pwrite, &pmsg->rawmsg.head, headpartlen);
        pwrite += headpartlen;
        memcpy(pwrite, pmsg->rawmsg.content, pmsg->rawmsg.clen);
        pwrite += pmsg->rawmsg.clen;
        //CRC16
        memcpy(pwrite, &pmsg->rawmsg.crc16, sizeof(pmsg->rawmsg.crc16));
        
        return (int)totallen & NETMESSAGE_HEAD_MASK;
    }
}

Msg* ResloveNetMessage(
        const unsigned char* netmsgbuf, const unsigned int buflen
        )
{
    char alias[MAX_ALIAS_LEN];
    net_msg_len_t totallen;

    const unsigned char* pread = netmsgbuf;
    memcpy(&totallen, pread, NETMESSAGE_HEADLEN); pread += NETMESSAGE_HEADLEN;
    if ((unsigned int)totallen > buflen){
        return NULL; // No more data to reslove
    }else{
        char from[ALIAS_LEN], to[ALIAS_LEN];
        RawMsg msg;
            
        memset(alias, 0, sizeof(alias));
        strcpy(alias, pread);
        pread += strlen(alias)+1;
        strncpy(from, alias, ALIAS_LEN);
            
        memset(alias, 0, sizeof(alias));
        strcpy(alias, pread);
        pread += strlen(alias)+1;
        strncpy(to, alias, ALIAS_LEN);

        int headpartlen = offsetof(message_t, content);
        memcpy(&msg, pread, headpartlen);
        pread += headpartlen;
        memcpy(msg.content, pread, msg.clen);
        pread += msg.clen;
        memcpy(&msg.crc16, pread, sizeof(msg.crc16));
        return new Msg(from, to, &msg);
    }
}

