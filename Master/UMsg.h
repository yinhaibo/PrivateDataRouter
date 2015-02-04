//---------------------------------------------------------------------------

#ifndef UMsgH
#define UMsgH

#include <system.hpp>
//---------------------------------------------------------------------------
// Raw Message define
#define ALIAS_LEN 5

#include "UComm.h"

//NET MESSAGE DEFINES
// MESSAGE STRUCTURE
//[ HEAD | LEN | FROM | TO | DATA ]


//NET MESSAGE MACRO DEFINES
#define NET_MESSAGE_HEAD 0x7E
#define NET_MESSAGE_HEAD_LEN     1
#define FILL_NET_MESSAGE_HEAD(buf) \
    do{ \
        *(unsigned char*)buf = NET_MESSAGE_HEAD; \
    }while(0)
#define IS_NET_MESSAGE_HEAD(buf) \
    (*(unsigned char*)buf == NET_MESSAGE_HEAD)


// Two bytes(A unsigend short int, max:65536)
#define NETMESSAGE_LEN_LEN 2
#define NETMESSAGE_LEN_MASK 0xFFFF
#define FILL_NETMESSAGE_LEN(buf, len) \
    do{ \
        ((unsigned char*)(buf))[0] = (len) & 0xFF; \
        ((unsigned char*)(buf))[1] = ((len) << 8) & 0xFF; \
    }while(0)
    
#define NETMESSAGE_LEN(buf, offset) \
      (*((unsigned char*)(buf) + offset) \
    + (*((unsigned char*)(buf) + offset + 1) << 8))
typedef unsigned short net_msg_len_t;

#define NETMESSAGE_MIN_LEN \
    (NET_MESSAGE_HEAD_LEN + NETMESSAGE_LEN_LEN + (1 + 1) * 2 + MIN_MESSAGE_LEN)
//
#define NETMESSAGE_MAX_LEN \
    (NET_MESSAGE_HEAD_LEN + NETMESSAGE_LEN_LEN + (ALIAS_LEN + 1) * 2 + MAX_MESSAGE_LEN)

// MASTER PACKAGE THE RAW MESSAGE A STREAM
// AND DO NOT PARSE THE RAW MESSAGE
// MASTER GENERAGE ERROR DATA IN STREAM WHEN
// RECEIVED OR SEND ON DEVICE INTERFACE AS
// RANDOM ERROR IN TRANSFER LINE DO

typedef struct RawMsg
{
    unsigned short len;
    unsigned char stream[MAX_MESSAGE_LEN];
}RawMsg;

class Msg{
public:
    Msg();
    Msg(char* from, char* to, const unsigned char* msg, unsigned short len);
    Msg(char* from, char* to, const RawMsg* msg);

    RawMsg rawmsg;
    char from[ALIAS_LEN+1];
    char to[ALIAS_LEN+1];
};


//---------------------------------------------------------------------------
#endif
