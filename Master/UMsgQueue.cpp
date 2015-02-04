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

////////////////////////////////////////////////////////////
// MASTER PACKAGE THE RAW MESSAGE A STREAM
// AND DO NOT PARSE THE RAW MESSAGE
// MASTER GENERAGE ERROR DATA IN STREAM WHEN
// RECEIVED OR SEND ON DEVICE INTERFACE AS
// RANDOM ERROR IN TRANSFER LINE DO

int BuildNetMessage(
        const Msg* pmsg,
        unsigned char* netmsgbuf, unsigned int buflen)
{
    /** net message length: NETMESSAGE_HEADLEN bytes + from alias length(include end char)
        + to alias bytes(include end char) + message
    **/
    if (pmsg == NULL) return -1;
    net_msg_len_t totallen = NET_MESSAGE_HEAD_LEN + NETMESSAGE_LEN_LEN
        + 1 + strlen(pmsg->from) + 1 + strlen(pmsg->to)
        + pmsg->rawmsg.len;
    if (buflen < (unsigned int)totallen){
        return -1;
    }else{
        unsigned char* pwrite = netmsgbuf;
        // write message head
        FILL_NET_MESSAGE_HEAD(pwrite);
        pwrite += NET_MESSAGE_HEAD_LEN;

        // write message length
        FILL_NETMESSAGE_LEN(pwrite, totallen);
        pwrite += NETMESSAGE_LEN_LEN;
        
        // write from and to alias
        strcpy(pwrite, pmsg->from);
        pwrite += strlen(pmsg->from) + 1;
        strcpy(pwrite, pmsg->to);
        pwrite += strlen(pmsg->to) + 1;

        // write message
        memcpy(pwrite, pmsg->rawmsg.stream, pmsg->rawmsg.len);
        
        return (int)totallen & NETMESSAGE_LEN_MASK;
    }
}

Msg* ResloveNetMessage(
        const unsigned char* netmsgbuf, const unsigned int buflen
        )
{

    net_msg_len_t totallen;
    
    size_t szMsgLen;
    const unsigned char* pread = netmsgbuf;

    // read head
    if(!IS_NET_MESSAGE_HEAD(pread)){
        return NULL; // Error head tag
    }
    pread += NET_MESSAGE_HEAD_LEN;

    // read length
    totallen = NETMESSAGE_LEN(pread, 0);
    szMsgLen = totallen - NET_MESSAGE_HEAD_LEN - NETMESSAGE_LEN_LEN; // Need to minus head and alias length
    pread += NETMESSAGE_LEN_LEN;
    if ((unsigned int)totallen > buflen){
        return NULL; // No more data to reslove
    }else{

        // read from and to alias(null-terminate)
        char alias[ALIAS_LEN+1];
        char from[ALIAS_LEN+1], to[ALIAS_LEN+1];

        size_t szAliasLen;
        memset(from, 0, sizeof(alias));
        strncpy(alias, pread, sizeof(alias));
        szAliasLen = strlen(alias) + 1;
        pread += szAliasLen;
        szMsgLen -= szAliasLen;
        strncpy(from, alias, ALIAS_LEN+1);
            
        memset(alias, 0, sizeof(alias));
        strncpy(alias, pread, sizeof(alias));
        szAliasLen = strlen(alias) + 1;
        pread += szAliasLen;
        szMsgLen -= szAliasLen;
        strncpy(to, alias, ALIAS_LEN);

        return  new Msg(from, to, pread, szMsgLen);
    }
}

