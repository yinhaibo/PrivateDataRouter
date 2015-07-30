//---------------------------------------------------------------------------


#pragma hdrstop
#include <stdio.h>
#include "UController.h"
#include "LogFileEx.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

extern LogFileEx logger;

///////////////////////////////////////////////////////////////////////////////
//Channel
///////////////////////////////////////////////////////////////////////////////
Channel::Channel()
{
    memset(alias, 0, sizeof(alias));
    executer = NULL;
    opened = false;

    priority = 0;
}

int Channel::getPriority()
{
    return priority;
}

void Channel::setPriority(const int val)
{
    priority = val;
    char buf[50];
    snprintf(buf, 50, "%s update priority %d", aliasString.c_str(), priority);
    logger.Log(buf);
}

bool Channel::isOpen()
{
    return opened;
}
void Channel::Open()
{
    opened = true;
}
void Channel::Close()
{
    opened = false;
}
IMsgPush* Channel::getExecuter()
{
    return executer;
}
void Channel::setExecuter(IMsgPush* executer)
{
    this->executer = executer;
}
int Channel::setAlias(const char* aliasString)
{
    memset(alias, 0, sizeof(alias));
    strcpy(alias[0], aliasString);
    this->aliasString = alias[0];
    return 1;
}
int Channel::setAlias(const char** aliasData)
{
    const char* pline;
    int count = 0;
    aliasString = "";
    memset(alias, 0, sizeof(alias));
    for (int i = 0; i < MAX_ALIAS_CNT; i++){
        pline = ((char*)aliasData + i * (ALIAS_LEN+1));
        if (pline[0] == '\0') break;
        strcpy(alias[i], pline);
        aliasString += alias[i];
        count++;
    }

    return count;
}
int Channel::setAlias(const AnsiString& aliasString)
{
    if (aliasString == NULL) return 0;
    memset(alias, 0, sizeof(alias));
    int splitpos = aliasString.AnsiPos(",");

    if (splitpos > 0){
        // maybe like ,A001,A002,A003
        AnsiString src = aliasString;
        int splitidx = 0;
        int prepos = 0;
        for (int idx = 0; idx < src.Length(); idx++){
            if (src.c_str()[idx] == ','){
                AnsiString portStr = aliasString.SubString(prepos+1, idx - prepos);
                strcpy(alias[splitidx], portStr.c_str());
                prepos = idx + 1;
                splitidx++;
            }
        }
        if (prepos < src.Length()){
            AnsiString portStr = aliasString.SubString(prepos+1, src.Length() - prepos);
            strcpy(alias[splitidx], portStr.c_str());
            splitidx++;
        }

        this->aliasString = aliasString;
        return splitidx;
    }else{
        strcpy(alias[0], aliasString.c_str());
        this->aliasString = aliasString;
        return 1;
    }
}
char** Channel::getAliasData()
{
    return (char**)alias;
}
bool Channel::hasAlias(const char* alias)
{
    for (int i = 0; i < MAX_ALIAS_CNT; i++){
        if (strcmp(alias, this->alias[i]) == 0){
            return true;
        }
    }
    if (this->alias[0][0] == '*'){
        return true; // match all other mode
    }
    return false;
}
#ifdef ENABLE_PRORITY
void Channel::incPriority()
{
    priority++;
    if (priority == 0x80000000) priority = 0x0FFFFFFF;
    #ifdef _DEBUG
    char buf[50];
    snprintf(buf, 50, "%s Inc priority %d", aliasString.c_str(), priority);
    logger.Log(buf);
    #endif
}
void Channel::decPriority()
{
    priority--;
    if (priority == 0x80000000) priority = 0x80000000;
    #ifdef _DEBUG
    char buf[50];
    snprintf(buf, 50, "%s Dec priority %d", aliasString.c_str(), priority);
    logger.Log(buf);
    #endif
}
#endif

const AnsiString& Channel::getAliasString() const
{
    return this->aliasString;
}
///////////////////////////////////////////////////////////////////////////////
// Controller
///////////////////////////////////////////////////////////////////////////////
Controller::Controller()
{
    csWorkVar = new TCriticalSection();
    retransModel = RETRANS_MODE_NEXT; // default to retransfrom in the next channel
}

Controller::~Controller()
{
    delete csWorkVar;

    Channel* selectItem;
    list<Channel*>::iterator it;
    for(it = lstChannel.begin();
        it != lstChannel.end();
        ++it){
        selectItem = *it;
        delete selectItem;
    }
    lstChannel.clear();
    
}
Channel* Controller::registerChannel(const AnsiString& aliasString, IMsgPush* executer,
            int priority)
{
    Channel* channel = new Channel();
    channel->setAlias(aliasString);
    channel->setExecuter(executer);
    channel->setPriority(priority);


    lstChannel.push_back(channel);

    return channel;
}

void Controller::unregisterChannel(Channel* channel)
{
    lstChannel.remove(channel);
    delete channel;
}

Channel* Controller::dispatchMsg(Msg* pmsg)
{
    return dispatchMsg(NULL, pmsg);
}
Channel* Controller::dispatchMsg(Channel* channel, Msg* pmsg, Channel* lastch, bool newmsg)
{
    Channel* rv = 0;
    if (csWorkVar != NULL){
        try{
            csWorkVar->Enter();
            rv = dispatchMsgSafe(channel, pmsg, lastch, newmsg);
        }__finally{
            csWorkVar->Leave();
        }
    }else{
        rv = dispatchMsgSafe(channel, pmsg, lastch, newmsg);
    }

    return rv;
}

Channel* Controller::getDispatchMsgCh(char* dest, Channel* lastch, bool newmsg)
{

    if (newmsg){
        return getNextDispatchMsgCh(dest, lastch);
    }else{
        switch (retransModel){
            case RETRANS_MODE_SAME: // retransmission in the same channel
                return lastch;
            case RETRANS_MODE_NEXT: // retransmission in the next channel
            default:
                return getNextDispatchMsgCh(dest, lastch);
        }
    }
}

Channel* Controller::getNextDispatchMsgCh(char* dest, Channel* lastch)
{
    Channel* selectCh = NULL;
    Channel* currentCh;
    #ifdef ENABLE_PRORITY
    int maxPriority = 0x80000000; //Min priority
    #endif
    list<Channel*>::iterator it;
    bool selectNext = false;
    
    for (it = lstChannel.begin();
         it != lstChannel.end();
         ++it){
         currentCh = *it;
         if (currentCh != NULL && currentCh->hasAlias(dest)){
            if (selectCh == NULL){
                // set first channel to selected channel
                selectCh = currentCh;
            }
            if (selectNext){
                selectCh = currentCh;
                break;
            }
            if (lastch == currentCh){
                #ifdef ENABLE_PRORITY
                // find the channel which have max priority less that current CH
                if (lastch != currentCh &&
                    currentCh->getPriority() <= lastch->getPriority()){
                    if (currentCh->getPriority() > maxPriority){
                        maxPriority = currentCh->getPriority();
                        selectCh = currentCh;
                    }
                }
                #else
                // one by one
                selectNext = true;
                #endif
            }
        }
    }

    return selectCh;
}

Channel* Controller::dispatchMsgSafe(Channel* channel, Msg* pmsg, Channel* lastch, bool newmsg)
{
    AnsiString dest = AnsiString(pmsg->to);
    Channel* selectCh = NULL;
    bool dispatchSucc = false;
    
    if (pmsg->msgtype == MSGTYPE_DATA){
        selectCh = getDispatchMsgCh(dest.c_str(), lastch, newmsg);

        if (selectCh != NULL){
            // allow send to channel which has highest priority
            // and the channel has opened
            if (selectCh->hasAlias(dest.c_str()) && selectCh->getExecuter()){
                if (selectCh->isOpen()){
                    #ifdef _DEBUG
                    LogMsg(channel, selectCh, pmsg, "push");
                    #endif
                    selectCh->getExecuter()->Push(pmsg);
                    dispatchSucc = true;
                }else{
                    #ifdef _DEBUG
                    LogMsg(channel, selectCh, pmsg, "not open");
                    #endif
                }
            }else{
                #ifdef _DEBUG
                LogMsg(channel, selectCh, pmsg, "no dest or executer");
                #endif
            }
        }else{
            #ifdef _DEBUG
            LogMsg(channel, NULL, pmsg, "no channel");
            #endif
        }
    }else if (pmsg->msgtype == MSGTYPE_TAGLIST){
        // Bind a new dest location set.
//        channel->setAlias((const char**)pmsg->taglist);
        if (channel->isOpen()){
            channel->getExecuter()->Push(pmsg);
            #ifdef _DEBUG
            LogMsg(channel, NULL, pmsg, "push taglist");
            #endif
            dispatchSucc = true;
        }else{
            #ifdef _DEBUG
            LogMsg(channel, NULL, pmsg, "not open");
            #endif
        }
        selectCh = channel;
    }

    if (!dispatchSucc){
        // !!!Message discard...
        #ifdef _DEBUG
        char buff[40];
        snprintf(buff, 40, "discard Msg:%08ul", pmsg->msgid);
        logger.Log(buff);
        #endif
        delete pmsg;
    }

    return selectCh;
}

void Controller::LogMsg(const Channel* fromch, const Channel* toch,
        const Msg* msg, AnsiString text)
{
    char buffer[200];
    if (fromch != NULL && toch != NULL && msg != NULL && text != NULL){
        snprintf(buffer, 200, "(%s)->(%s)-[%s->%s]:[%s]",
            fromch->getAliasString().c_str(),
            toch->getAliasString().c_str(),
            msg->from, msg->to, text.c_str());
    }else if(fromch != NULL && msg != NULL){
        snprintf(buffer, 200, "(%s)->(%s)-[%s->%s]:[%s]",
            fromch->getAliasString().c_str(),
            "null",
            msg->from, msg->to, text.c_str());
    }

    logger.Log(buffer);
}
#ifdef ENABLE_PRIORITY
bool Controller::incChannelPriority(Channel* channel)
{
    Channel* selectItem;
    if (channel == NULL) return false;
    list<Channel*>::iterator it;
    try{
        csWorkVar->Enter();
        for(it = lstChannel.begin();
            it != lstChannel.end();
            ++it){
            selectItem = *it;
            if (channel == selectItem){
                channel->incPriority();
                return true;
            }
        }
    }__finally{
        csWorkVar->Leave();
    }
    return false;
}

bool Controller::decChannelPriority(Channel* channel)
{
    Channel* selectItem;
    if (channel == NULL) return false;
    list<Channel*>::iterator it;
    try{
        csWorkVar->Enter();
        for(it = lstChannel.begin();
            it != lstChannel.end();
            ++it){
            selectItem = *it;
            if (channel == selectItem){
                #ifdef ENABLE_PRIORITY
                channel->decPriority();
                #endif
                return true;
            }
        }
    }__finally{
        csWorkVar->Leave();
    }
    return false;
}
#endif

void Controller::setRetransMode(RETRANS_MODE mode)
{
    retransModel = mode;
}



