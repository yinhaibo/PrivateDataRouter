//---------------------------------------------------------------------------


#pragma hdrstop

#include "UMsgQueue.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)



///////////////////////////////////////////////////
// MsgQueue
///////////////////////////////////////////////////
MsgQueue::MsgQueue()
{
    mQueue = new TThreadList();
}

MsgQueue::~MsgQueue()
{
    Clear();
    delete mQueue;
}

void MsgQueue::Push(Msg* pmsg)
{
    mQueue->Add(pmsg);
}

Msg* MsgQueue::Pop()
{
    if (!Empty()){
        try{
            TList* lst = mQueue->LockList();
            Msg *pmsg = (Msg*)lst->First();
            lst->Delete(0);
            if (pmsg != NULL){
                return pmsg;
            }
        }__finally{
            mQueue->UnlockList();
        }
    }
    return NULL;
}
void MsgQueue::Clear()
{
    try{
        TList *list = mQueue->LockList();
        for (int i = 0; i < list->Count; i++)
        {
            delete (Msg *)list->Items[i];
        }
        list->Clear();
    }__finally{
        mQueue->UnlockList();
    }
}
bool MsgQueue::Empty() const
{
    bool bEmpty = true;
    try{
        TList *list = mQueue->LockList();
        bEmpty = (0 == list->Count);
    }__finally{
        mQueue->UnlockList();
    }
    return bEmpty;
}

///////////////////////////////////////////////////
// RawMsgQueue
///////////////////////////////////////////////////
RawMsgQueue::RawMsgQueue()
{
    mQueue = new TThreadList();
}

RawMsgQueue::~RawMsgQueue()
{
    Clear();
    delete mQueue;
}

void RawMsgQueue::Push(RawMsg* pmsg)
{
    mQueue->Add(pmsg);
}

RawMsg* RawMsgQueue::Pop()
{
    if (!Empty()){
        try{
            TList* lst = mQueue->LockList();
            RawMsg *pmsg = (RawMsg*)lst->First();
            lst->Delete(0);
            if (pmsg != NULL){
                return pmsg;
            }
        }__finally{
            mQueue->UnlockList();
        }
    }
    return NULL;
}

void RawMsgQueue::Clear()
{
    try{
        TList *list = mQueue->LockList();
        for (int i = 0; i < list->Count; i++)
        {
            delete (Msg *)list->Items[i];
        }
        list->Clear();
    }__finally{
        mQueue->UnlockList();
    }
}

bool RawMsgQueue::Empty() const
{
    bool bEmpty = true;
    try{
        TList *list = mQueue->LockList();
        bEmpty = (0 == list->Count);
    }__finally{
        mQueue->UnlockList();
    }
    return bEmpty;
}

