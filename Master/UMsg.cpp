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
    memset(rawmsg.message, 0, sizeof(rawmsg.message));
    memset(from, 0, ALIAS_LEN);
    memset(to, 0, ALIAS_LEN);
}

Msg::Msg(char* from, char* to, const RawMsg& msg)
{
    strncpy(this->from, from, ALIAS_LEN);
    strncpy(this->to, to, ALIAS_LEN);
    memcpy(rawmsg.message, msg.message, sizeof(msg.message));
}