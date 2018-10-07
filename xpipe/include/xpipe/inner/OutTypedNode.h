#ifndef XPIPE_INNER_OUTTYPEDNODE_H
#define XPIPE_INNER_OUTTYPEDNODE_H

#include <algorithm>
#include <iterator>
#include <memory>
#include <cassert>
#include <vector>

#include "xpipe/inner/Nullable.h"
#include "xpipe/inner/Node.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<typename OUT>
        class InTypedNode;

        template<typename OUT>
        class OutTypedNode: public virtual Node
        {
        public:
            using OutType = OUT;
            using NodePtrCol = std::vector<graphptr::LinkPointer<Node>>;

        public:
            virtual Nullable<OutType> tryPop() = 0;
            virtual bool canPop() const = 0;

            const NodeCol &children() const override
            {
                return children_;
            }
            void clearChildren() override
            {
                children_.clear();
                nodes.clear();
            }
            bool childrenAreFinished() const override;

            void setChild(graphptr::LinkPointer<Node> child);
            void setChildren(const NodePtrCol &cs);

        protected:
            NodeCol children_;

        private:
            NodePtrCol nodes;
        };

        template<typename OUT>
        void OutTypedNode<OUT>::setChild(
            graphptr::LinkPointer<Node> child)
        {
            nodes = NodePtrCol{child};
            children_ = {child.get()};
        }

        template<typename Out>
        void OutTypedNode<Out>::setChildren(const NodePtrCol &cs)
        {
            nodes = cs;
            std::transform(std::begin(nodes), std::end(nodes),
                std::back_inserter(children_),
                [](const NodePtrCol::value_type &n){
                    return n.get();
                });
        }

        template<typename Out>
        bool OutTypedNode<Out>::childrenAreFinished() const
        {
            if(children_.empty())
                return true;
            for(auto *c : children_)
            {
                assert(c);
                if(c->childrenAreFinished())
                    return true;
            }
            return false;
        }
    }
}

#endif
