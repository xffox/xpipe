#ifndef XPIPE_INNER_SEQNODE_H
#define XPIPE_INNER_SEQNODE_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <cassert>

#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<typename OUT>
        class SeqNode: public OutTypedNode<OUT>
        {
        public:
            using NodePtrCol =
                std::vector<graphptr::LinkPointer<OutTypedNode<OUT>>>;

        public:
            SeqNode();

            SeqNode(const SeqNode&) = delete;
            SeqNode &operator=(const SeqNode&) = delete;

            void setParents(const NodePtrCol &parents);

            Task *task() override
            {
                return nullptr;
            }

            Nullable<OUT> tryPop() override;
            bool canPop() const override;
            bool parentsAreDone() const override;

            const Node::NodeCol &parents() const override
            {
                return parents_;
            }
            void clearParents() override
            {
                parents_.clear();
                prevs.clear();
                prevIdx = 0;
            }

        private:
            NodePtrCol prevs;
            Node::NodeCol parents_;
            std::size_t prevIdx = 0;
        };

        template<typename OUT>
        SeqNode<OUT>::SeqNode()
            :prevs(), parents_()
        {}

        template<typename Out>
        void SeqNode<Out>::setParents(const NodePtrCol &parents)
        {
            this->prevs = std::move(parents);
            std::transform(std::begin(parents), std::end(parents),
                std::back_inserter(parents_),
                [](const typename NodePtrCol::value_type &parent){
                    return parent.get();
                });
        }

        template<typename OUT>
        Nullable<OUT> SeqNode<OUT>::tryPop()
        {
            assert(!prevs.empty());
            while(prevIdx < prevs.size())
            {
                if(!prevs[prevIdx]->parentsAreDone())
                {
                    return prevs[prevIdx]->tryPop();
                }
                ++prevIdx;
            }
            return Nullable<OUT>();
        }

        template<typename OUT>
        bool SeqNode<OUT>::canPop() const
        {
            assert(!prevs.empty());
            for(auto i = prevIdx; i < prevs.size(); ++i)
            {
                if(prevs[i]->canPop())
                    return true;
                if(!prevs[i]->parentsAreDone())
                    return false;
            }
            return false;
        }

        template<typename OUT>
        bool SeqNode<OUT>::parentsAreDone() const
        {
            assert(!prevs.empty());
            for(auto i = prevIdx; i < prevs.size(); ++i)
            {
                if(!prevs[i]->parentsAreDone())
                    return false;
            }
            return true;
        }
    }
}

#endif
