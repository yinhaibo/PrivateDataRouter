//---------------------------------------------------------------------------

#ifndef UMsgH
#define UMsgH

#include <system.hpp>
//---------------------------------------------------------------------------
// Raw Message define
#define MESSAGE_LEN 5
#define ALIAS_LEN 5

#include "UComm.h"

// Two bytes(A unsigend short int, max:65536)
#define NETMESSAGE_HEADLEN 2
#define NETMESSAGE_HEAD_MASK 0xFFFF
#define NETMESSAGE_HEAD(buf, offset) \
      (*((unsigned char*)(buf) + offset) \
    + (*((unsigned char*)(buf) + offset + 1) << 8))
typedef unsigned short net_msg_len_t;
#define MAX_ALIAS_LEN 20

typedef message_t RawMsg;
class Msg{
public:
    Msg();
    Msg(char* from, char* to, const RawMsg* msg);

    RawMsg rawmsg;
    char from[ALIAS_LEN];
    char to[ALIAS_LEN];
};


//---------------------------------------------------------------------------
#endif
