//---------------------------------------------------------------------------


#pragma hdrstop

#include "UMsgQueue.h"
#include "Tools.h"
#include "UMsg.h"

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
    net_msg_len_t totallen;

    if (pmsg->msgtype == MSGTYPE_DATA){
        totallen = NET_MESSAGE_HEAD_LEN + NETMESSAGE_LEN_LEN + NET_MESSAGE_CMD_LEN
            + 1 + strlen(pmsg->from) + 1 + strlen(pmsg->to)
            + pmsg->rawmsg.len + NETMESSAGE_CRC_LEN;
    }else{
        totallen = NET_MESSAGE_HEAD_LEN + NETMESSAGE_LEN_LEN
            + NET_MESSAGE_CMD_LEN + NETMESSAGE_CRC_LEN;
        for (int i = 0; i < MAX_ALIAS_CNT; i++){
                if (pmsg->taglist[i][0] != '\0'){
                    totallen += strlen(pmsg->taglist[i]) + 1;
                }else{
                    break;
                }
        }
    }
    if (buflen < (unsigned int)totallen){
        return -1;
    }else{
        unsigned char* pwrite = netmsgbuf;
        unsigned short crcVal;
        // write message head
        FILL_NET_MESSAGE_HEAD(pwrite);
        pwrite += NET_MESSAGE_HEAD_LEN;


        if (pmsg->msgtype == MSGTYPE_DATA){
            // write message command
            FILL_NET_MESSAGE_CMD(pwrite, NET_MESSAGE_CMD_IODATA);
            pwrite += NET_MESSAGE_CMD_LEN;

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
            pwrite += pmsg->rawmsg.len;

            // write CRC
            crcVal = crc16_cal(netmsgbuf, totallen - NETMESSAGE_CRC_LEN);
            FILL_NETMESSAGE_CRC(pwrite, 0, crcVal);
        }else if(pmsg->msgtype == MSGTYPE_TAGLIST){
            // write message command
            FILL_NET_MESSAGE_CMD(pwrite, NET_MESSAGE_CMD_TAGLIST);
            pwrite += NET_MESSAGE_CMD_LEN;

             // write message length
            FILL_NETMESSAGE_LEN(pwrite, totallen);
            pwrite += NETMESSAGE_LEN_LEN;

            // write message
            for (int i = 0; i < MAX_ALIAS_CNT; i++){
                if (pmsg->taglist[i][0] == '\0') break;
                
                strcpy(pwrite, pmsg->taglist[i]);
                pwrite += strlen(pmsg->taglist[i]) + 1;
            }
            // write CRC
            crcVal = crc16_cal(netmsgbuf, totallen - NETMESSAGE_CRC_LEN);
            FILL_NETMESSAGE_CRC(pwrite, 0, crcVal);
        }
        
        return (int)totallen & NETMESSAGE_LEN_MASK;
    }
}

Msg* ResloveNetMessage(
        const unsigned char* netmsgbuf, const unsigned int buflen
        )
{

    net_msg_len_t totallen;
    net_message_cmd_t cmd;
    
    size_t szMsgLen;
    const unsigned char* pread = netmsgbuf;

    // check crc
    unsigned short usCrcCal;
    unsigned short usCrcData;
    usCrcCal = crc16_cal((unsigned char*)netmsgbuf, buflen - NETMESSAGE_CRC_LEN);
    usCrcData = GET_NETMESSAGE_CRC(netmsgbuf, buflen - NETMESSAGE_CRC_LEN);
    if (usCrcCal != usCrcData){
        // commit by yhb in 2015/03/30 19:52
        // there is meaningless to reslove a error CRC record.
        // Msg *pmsg = new Msg(false);
        return NULL;
    }
    // read head
    if(!IS_NET_MESSAGE_HEAD(pread)){
        return NULL; // Error head tag
    }
    pread += NET_MESSAGE_HEAD_LEN;

    // read command
    if(!IS_NET_MESSAGE_CMD(pread, 0)){
        return NULL; // Error head tag
    }
    cmd = NET_MESSAGE_CMD(pread, 0);
    pread += NET_MESSAGE_CMD_LEN;

    // read length
    totallen = NETMESSAGE_LEN(pread, 0);
    // Need to minus head and alias length
    szMsgLen = totallen - NET_MESSAGE_HEAD_LEN - NETMESSAGE_LEN_LEN
         - NET_MESSAGE_CMD_LEN - NETMESSAGE_CRC_LEN;
    pread += NETMESSAGE_LEN_LEN;
    if ((unsigned int)totallen > buflen){
        return NULL; // No more data to reslove
    }else{
        if (CMP_NET_MESSAGE_CMD(cmd, NET_MESSAGE_CMD_IODATA)){
            // from alias + '\0' + to alias + '\0' + message
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

            Msg* pmsg = new Msg(from, to, pread, szMsgLen);
            return pmsg;
        }else if(CMP_NET_MESSAGE_CMD(cmd, NET_MESSAGE_CMD_TAGLIST)){
            //alias + '\0' + alias + '\0' + ...
            int idx = 0;
            Msg *pmsg = new Msg();
            while(pread < netmsgbuf + buflen - NETMESSAGE_CRC_LEN){
                strcpy(pmsg->taglist[idx], pread);
                pread += strlen(pmsg->taglist[idx]) + 1;
                idx++;
                if (idx >= MAX_ALIAS_CNT) break;
            }
            return pmsg;
        }
        return NULL;
    }
}

