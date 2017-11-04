/**
   @file
   @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_BASE_ABSTRACT_SEQ_ITEM_H
#define CNOID_BASE_ABSTRACT_SEQ_ITEM_H

#include "Item.h"
#include <cnoid/AbstractSeq>
#include "exportdecl.h"

namespace cnoid {

class CNOID_EXPORT AbstractSeqItem : public Item
{
public:
    AbstractSeqItem();
    AbstractSeqItem(const AbstractSeqItem& org);
    virtual ~AbstractSeqItem();

#if _MSC_VER == 1900
    virtual AbstractSeqPtr abstractSeq() { return NULL; }
#else
    virtual AbstractSeqPtr abstractSeq() = 0;
#endif

protected:
    virtual void doPutProperties(PutPropertyFunction& putProperty);
    virtual bool store(Archive& archive);
    virtual bool restore(const Archive& archive);
};

typedef ref_ptr<AbstractSeqItem> AbstractSeqItemPtr;


class CNOID_EXPORT AbstractMultiSeqItem : public AbstractSeqItem
{
public:
    AbstractMultiSeqItem();
    AbstractMultiSeqItem(const AbstractMultiSeqItem& org);
    virtual ~AbstractMultiSeqItem();

    virtual AbstractSeqPtr abstractSeq();
#if _MSC_VER == 1900
    virtual AbstractMultiSeqPtr abstractMultiSeq() { return NULL; }
#else
    virtual AbstractMultiSeqPtr abstractMultiSeq() = 0;
#endif

protected:
    virtual void doPutProperties(PutPropertyFunction& putProperty);
};

typedef ref_ptr<AbstractMultiSeqItem> AbstractMultiSeqItemPtr;

}

#endif
