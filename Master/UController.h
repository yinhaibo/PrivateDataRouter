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
    void incPriority();
    void decPriority();
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
    virtual bool dispatchMsg(Channel* channel, Msg* pmsg) = 0;
    virtual bool dispatchMsg(Msg* pmsg) = 0;
};

class Controller : public IDispatcher
{
private:
    TCriticalSection* csWorkVar;
    list<Channel*> lstChannel;
    
    bool dispatchMsgSafe(Channel* channel, Msg* pmsg);

    void LogMsg(const Channel* fromch, const Channel* toch,
        const Msg* msg, AnsiString text);
public:
    virtual bool dispatchMsg(Channel* channel, Msg* pmsg);
    virtual bool dispatchMsg(Msg* pmsg);

    Channel* registerChannel(const AnsiString& aliasString, IMsgPush* executer,
            int priority);
    void unregisterChannel(Channel* channel);

    Controller();
    ~Controller();
};
//---------------------------------------------------------------------------
#endif
