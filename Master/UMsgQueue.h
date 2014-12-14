//---------------------------------------------------------------------------

#ifndef UMsgQueueH
#define UMsgQueueH

#include <Classes.hpp>

#include "UMsg.h"

class IMsgPush{
public:
    virtual void Push(Msg* pmsg) = 0;
};

class IRawMsgPush{
public:
    virtual void Push(RawMsg* pmsg) = 0;
};

class IQueue : public IMsgPush
{
public:
    virtual void Push(Msg* pmsg) = 0;
    virtual Msg* Pop() = 0;
    virtual void Clear() = 0;
    virtual bool Empty() const = 0;
};

class IRawQueue : public IRawMsgPush
{
public:
    virtual void Push(RawMsg* pmsg) = 0;
    virtual RawMsg* Pop() = 0;
    virtual void Clear() = 0;
    virtual bool Empty() const = 0;
};

class MsgQueue : public IQueue
{
private:
    TThreadList* mQueue;
public:
    MsgQueue();
    ~MsgQueue();
    virtual void Push(Msg* pmsg);
    virtual Msg* Pop();
    virtual void Clear();
    virtual bool Empty() const;
};

class RawMsgQueue : public IRawQueue
{
private:
    TThreadList* mQueue;
public:
    RawMsgQueue();
    ~RawMsgQueue();
    virtual void Push(RawMsg* pmsg);
    virtual RawMsg* Pop();
    virtual void Clear();
    virtual bool Empty() const;
};
//---------------------------------------------------------------------------
#endif
 