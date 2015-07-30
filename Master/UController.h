//---------------------------------------------------------------------------

#ifndef UControllerH
#define UControllerH
#include <SyncObjs.hpp>
#include <map>
#include <list>
#include "UMsgQueue.h"
#include "UServerClientWorkThread.h"

using namespace std;


class Channel
{
private:
    char alias[MAX_ALIAS_CNT][ALIAS_LEN+1];
    AnsiString aliasString;
    IMsgPush* executer;
    bool opened;
    int  priority;
public:
    Channel();

    int getPriority();
    void setPriority(const int val);
#ifdef ENABLE_PRIORITY
    void incPriority();
    void decPriority();
#endif
    bool isOpen();
    void Open();
    void Close();
    IMsgPush* getExecuter();
    void setExecuter(IMsgPush* executer);
    int setAlias(const char* aliasString);
    int setAlias(const char** aliasData);
    int setAlias(const AnsiString& aliasString);
    char** getAliasData();
    const AnsiString& getAliasString() const;
    bool hasAlias(const char* alias);
};

class IDispatcher
{
public:
    virtual Channel* dispatchMsg(Channel* channel, Msg* pmsg, Channel* lastch = NULL, bool newmsg = true) = 0;
    virtual Channel* dispatchMsg(Msg* pmsg) = 0;
};

typedef enum _RETRANS_MODE{
    RETRANS_MODE_SAME, // retransmission in the same channel
    RETRANS_MODE_NEXT, // retransmission in the next channel
}RETRANS_MODE;

class Controller : public IDispatcher
{
private:
    TCriticalSection* csWorkVar;
    list<Channel*> lstChannel;
    RETRANS_MODE retransModel;
    
    Channel* dispatchMsgSafe(Channel* channel, Msg* pmsg, Channel* lastch, bool newmsg);
    Channel* getNextDispatchMsgCh(char* dest, Channel* lastch);

    void LogMsg(const Channel* fromch, const Channel* toch,
        const Msg* msg, AnsiString text);

    Channel* getDispatchMsgCh(char* dest, Channel* lastch = NULL, bool newmsg = true);
public:
    virtual Channel* dispatchMsg(Channel* channel, Msg* pmsg, Channel* lastch = NULL, bool newmsg = true);
    virtual Channel* dispatchMsg(Msg* pmsg);

    Channel* registerChannel(const AnsiString& aliasString, IMsgPush* executer,
            int priority);
    void unregisterChannel(Channel* channel);
    bool incChannelPriority(Channel* channel);
    bool decChannelPriority(Channel* channel);

    void setRetransMode(RETRANS_MODE mode);

    Controller();
    ~Controller();
};
//---------------------------------------------------------------------------
#endif
