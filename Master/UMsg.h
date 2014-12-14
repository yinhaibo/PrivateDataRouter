//---------------------------------------------------------------------------

#ifndef UMsgH
#define UMsgH

#include <system.hpp>
//---------------------------------------------------------------------------
// Raw Message define
#define MESSAGE_LEN 5
#define ALIAS_LEN 5

typedef struct RawMsg{
    unsigned char message[MESSAGE_LEN];
}RawMsg;

class Msg{
public:
    Msg();
    Msg(char* from, char* to, const RawMsg& msg);

    RawMsg rawmsg;
    char from[ALIAS_LEN];
    char to[ALIAS_LEN];
};


//---------------------------------------------------------------------------
#endif
