#ifndef XPIPE_INNER_PARNODE_H
#define XPIPE_INNER_PARNODE_H

#include <cassert>

#include "xpipe/inner/InTypedNode.h"
#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<typename T>
        class ParNode: public InTypedNode<T>,
            public OutTypedNode<T>
        {

        public:
            Nullable<T> tryPop() override;
            bool canPop() const override;
            bool childrenAreFinished() const override;

            Task *task() override
            {
                return nullptr;
            }
        };

        template<typename In>
        Nullable<In> ParNode<In>::tryPop()
        {
            return ParNode::parentTryPop();
        }

        template<typename In>
        bool ParNode<In>::canPop() const
        {
            return ParNode::parentCanPop();
        }

        template<typename In>
        bool ParNode<In>::childrenAreFinished() const
        {
            for(const auto *c : ParNode::children_)
            {
                assert(c);
                if(!c->childrenAreFinished())
                    return false;
            }
            return true;
        }

    }
}

#endif
