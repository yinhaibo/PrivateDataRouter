//---------------------------------------------------------------------------


#pragma hdrstop

#include "UMsg.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

///////////////////////////////////////////////////
// Msg
///////////////////////////////////////////////////
static LONG volatile msgid = 0;
Msg::Msg()
{
    memset(&rawmsg, 0, sizeof(RawMsg));
    memset(from, 0, ALIAS_LEN);
    memset(to, 0, ALIAS_LEN);
    memset(taglist, 0, sizeof(taglist));

    msgtype = MSGTYPE_TAGLIST;
    validedOK = true;

    InterlockedIncrement(&msgid);
    this->msgid = msgid;
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

    InterlockedIncrement(&msgid);
    this->msgid = msgid;
}

Msg::Msg(char* from, char* to, const RawMsg* msg)
{
    validedOK = true;
    msgtype = MSGTYPE_DATA;
    strncpy(this->from, from, ALIAS_LEN);
    strncpy(this->to, to, ALIAS_LEN);
    memcpy(&rawmsg, msg, sizeof(RawMsg));
    InterlockedIncrement(&msgid);
    this->msgid = msgid;
}

Msg::Msg(bool valided) : validedOK(valided)
{
    memset(&rawmsg, 0, sizeof(RawMsg));
    memset(from, 0, ALIAS_LEN);
    memset(to, 0, ALIAS_LEN);
    memset(taglist, 0, sizeof(taglist));

    msgtype = MSGTYPE_DATA;
    InterlockedIncrement(&msgid);
    this->msgid = msgid;
}

///////////////////////////////////////////////////
// RawMsg
///////////////////////////////////////////////////
RawMsg::RawMsg(){
    this->len = 0;
    memset(stream, 0, MAX_MESSAGE_LEN);
}
RawMsg::RawMsg(const RawMsg& value){
    this->len = value.len;
    memcpy(this->stream, value.stream, MAX_MESSAGE_LEN);
}
RawMsg::RawMsg(RawMsg* value){
    this->len = value->len;
    memcpy(this->stream, value->stream, MAX_MESSAGE_LEN);
}

