//---------------------------------------------------------------------------

#ifndef UMsgH
#define UMsgH

#include <system.hpp>
//---------------------------------------------------------------------------
// Raw Message define
#define ALIAS_LEN 5
// Number of max alias in every Master 
#define MAX_ALIAS_CNT 10

#include "UComm.h"

//NET MESSAGE DEFINES
// MESSAGE STRUCTURE
//[ HEAD | LEN | FROM | TO | DATA ]


//NET MESSAGE MACRO DEFINES
#define NET_MESSAGE_HEAD 0x7E
#define NET_MESSAGE_HEAD_LEN     1
#define FILL_NET_MESSAGE_HEAD(buf) \
    do{ \
        *(unsigned char*)(buf) = NET_MESSAGE_HEAD; \
    }while(0)
#define IS_NET_MESSAGE_HEAD(buf) \
    (*(unsigned char*)buf == NET_MESSAGE_HEAD)

#define NET_MESSAGE_CMD_LEN      1
typedef unsigned char net_message_cmd_t;
#define NET_MESSAGE_CMD_TAGLIST 0x01
#define NET_MESSAGE_CMD_IODATA  0x02
#define NET_MESSAGE_CMD_ERRCMD  0xE0
#define FILL_NET_MESSAGE_CMD(buf, cmd) \
    do{ \
        *(unsigned char*)(buf) = (cmd); \
    }while(0)
#define IS_NET_MESSAGE_CMD(buf, offset) \
    ((((unsigned char*)(buf))[offset] == NET_MESSAGE_CMD_TAGLIST) ||  \
     (((unsigned char*)(buf))[offset] == NET_MESSAGE_CMD_IODATA) ||   \
     (((unsigned char*)(buf))[offset] == NET_MESSAGE_CMD_ERRCMD))
#define NET_MESSAGE_CMD(buf, offset) \
    (((unsigned char*)(buf))[offset])
#define CMP_NET_MESSAGE_CMD(cmd1, cmd2)  (cmd1 == cmd2)
    
// Two bytes(A unsigend short int, max:65536)
#define NETMESSAGE_LEN_LEN 2
#define NETMESSAGE_LEN_MASK 0xFFFF
#define FILL_NETMESSAGE_LEN(buf, len) \
    do{ \
        ((unsigned char*)(buf))[0] = (len) & 0xFF; \
        ((unsigned char*)(buf))[1] = ((len) >> 8) & 0xFF; \
    }while(0)
    
#define NETMESSAGE_LEN(buf, offset) \
      (*((unsigned char*)(buf) + (offset)) \
    + (*((unsigned char*)(buf) + (offset) + 1) << 8))
typedef unsigned short net_msg_len_t;

#define NETMESSAGE_CRC_LEN 2
#define FILL_NETMESSAGE_CRC(buf, offset, crc) \
    do{ \
        ((unsigned char*)(buf))[(offset) + 0] = (crc) & 0xFF; \
        ((unsigned char*)(buf))[(offset) + 1] = ((crc) >> 8) & 0xFF; \
    }while(0)
#define GET_NETMESSAGE_CRC(buf, offset) \
    (*((unsigned char*)(buf) + (offset)) \
    + (*((unsigned char*)(buf) + (offset) + 1) << 8))

#define NETMESSAGE_MIN_LEN \
    (NET_MESSAGE_HEAD_LEN + NET_MESSAGE_CMD_LEN + NETMESSAGE_LEN_LEN \
    + 2 + NETMESSAGE_CRC_LEN)
//
#define NETMESSAGE_MAX_LEN \
    (NET_MESSAGE_HEAD_LEN + NET_MESSAGE_CMD_LEN + NETMESSAGE_LEN_LEN \
    + (ALIAS_LEN + 1) * MAX_ALIAS_CNT + MAX_MESSAGE_LEN + NETMESSAGE_CRC_LEN)

// MASTER PACKAGE THE RAW MESSAGE A STREAM
// AND DO NOT PARSE THE RAW MESSAGE
// MASTER GENERAGE ERROR DATA IN STREAM WHEN
// RECEIVED OR SEND ON DEVICE INTERFACE AS
// RANDOM ERROR IN TRANSFER LINE DO

typedef struct RawMsg
{
    unsigned short len;
    unsigned char stream[MAX_MESSAGE_LEN];
public:
    RawMsg(){
        this->len = 0;
        memset(stream, 0, MAX_MESSAGE_LEN);
    }
    RawMsg(const RawMsg& value){
        this->len = value.len;
        memcpy(this->stream, value.stream, MAX_MESSAGE_LEN);
    }
    RawMsg(RawMsg* value){
        this->len = value->len;
        memcpy(this->stream, value->stream, MAX_MESSAGE_LEN);
    }
}RawMsg;

typedef enum MsgType
{
    MSGTYPE_TAGLIST,
    MSGTYPE_DATA,
}MsgType;

class Msg{
public:
    Msg();
    Msg(char* from, char* to, const unsigned char* msg, unsigned short len);
    Msg(char* from, char* to, const RawMsg* msg);
    Msg(bool valided);
    Msg(const Msg& msg){
        memcpy(&this->rawmsg, &msg.rawmsg, sizeof(RawMsg));
        this->msgtype = msg.msgtype;
        memcpy(this->from, msg.from, ALIAS_LEN+1);
        memcpy(this->to, msg.to, ALIAS_LEN+1);
        this->validedOK = msg.validedOK;
        memcpy(this->taglist, msg.taglist, MAX_ALIAS_CNT * (ALIAS_LEN+1));
    }

    RawMsg rawmsg;
    MsgType msgtype;

    // For data
    char from[ALIAS_LEN+1];
    char to[ALIAS_LEN+1];
    bool validedOK;

    // For tag list
    char taglist[MAX_ALIAS_CNT][ALIAS_LEN+1];
};


//---------------------------------------------------------------------------
#endif
