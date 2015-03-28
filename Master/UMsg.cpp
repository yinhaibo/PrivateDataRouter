//---------------------------------------------------------------------------


#pragma hdrstop

#include "UMsg.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

///////////////////////////////////////////////////
// Msg
///////////////////////////////////////////////////
Msg::Msg()
{
    memset(&rawmsg, 0, sizeof(RawMsg));
    memset(from, 0, ALIAS_LEN);
    memset(to, 0, ALIAS_LEN);
    memset(taglist, 0, sizeof(taglist));

    msgtype = MSGTYPE_TAGLIST;
    validedOK = true;
}

Msg::Msg(char* from, char* to, const unsigned char* msg,
    unsigned short len)
{
    validedOK = true;
    msgtype = MSGTYPE_DATA;
    strncpy(this->from, from, ALIAS_LEN);
    strncpy(this->to, to, ALIAS_LEN);
    rawmsg.len = len;
    memcpy(rawmsg.stream, msg, len);
}

Msg::Msg(char* from, char* to, const RawMsg* msg)
{
    validedOK = true;
    msgtype = MSGTYPE_DATA;
    strncpy(this->from, from, ALIAS_LEN);
    strncpy(this->to, to, ALIAS_LEN);
    memcpy(&rawmsg, msg, sizeof(RawMsg));
}

Msg::Msg(bool valided) : validedOK(valided)
{
}
