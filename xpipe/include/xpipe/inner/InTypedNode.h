#ifndef XPIPE_INNER_INTYPEDNODE_H
#define XPIPE_INNER_INTYPEDNODE_H

#include <memory>

#include "xpipe/inner/Nullable.h"
#include "xpipe/inner/Node.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<typename OUT>
        class OutTypedNode;

        template<typename IN>
        class InTypedNode: public virtual Node
        {
        public:
            using InType = IN;

        public:
            InTypedNode()
                :parent(), parents_{}
            {}

            void setParent(graphptr::LinkPointer<OutTypedNode<InType>> parent);

            const NodeCol &parents() const override
            {
                return parents_;
            }

            void clearParents() override
            {
                parents_.clear();
                parent = decltype(parent)();
            }

        protected:
            Nullable<InType> parentTryPop()
            {
                if(parent)
                {
                    return parent->tryPop();
                }
                return Nullable<InType>();
            }

            bool parentCanPop() const
            {
                if(parent)
                {
                    return parent->canPop();
                }
                return false;
            }

            bool parentsAreDone() const override
            {
                if(parent)
                {
                    return parent->parentsAreDone();
                }
                return true;
            }

        private:
            graphptr::LinkPointer<OutTypedNode<InType>> parent;
            NodeCol parents_;
        };
    }
}

#include "xpipe/inner/OutTypedNode.h"

namespace xpipe
{
    namespace inner
    {
        template<typename IN>
        void InTypedNode<IN>::setParent(
            graphptr::LinkPointer<OutTypedNode<InType>> parent)
        {
            this->parent = parent;
            parents_ = {parent.get()};
        }
    }
}

#endif
