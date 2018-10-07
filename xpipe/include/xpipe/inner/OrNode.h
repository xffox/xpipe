#ifndef XPIPE_INNER_TYPEDTASK_H
#define XPIPE_INNER_TYPEDTASK_H

#include <cstddef>
#include <memory>
#include <cassert>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <functional>

#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<typename OUT>
        class OrNode: public OutTypedNode<OUT>
        {
        public:
            using NodePtrCol =
                std::vector<graphptr::LinkPointer<OutTypedNode<OUT>>>;

        public:
            OrNode();

            OrNode(const OrNode&) = delete;
            OrNode &operator=(const OrNode&) = delete;

            void setParents(const NodePtrCol &prevs);

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
        OrNode<OUT>::OrNode()
            :prevs(), parents_()
        {}

        template<typename Out>
        void OrNode<Out>::setParents(const NodePtrCol &prevs)
        {
            parents_.clear();
            this->prevs = prevs;
            std::transform(std::begin(prevs), std::end(prevs),
                std::back_inserter(parents_),
                [](const typename NodePtrCol::value_type &prev){
                    return prev.get();
                });
        }

        template<typename OUT>
        Nullable<OUT> OrNode<OUT>::tryPop()
        {
            assert(!prevs.empty());
            const auto sz = prevs.size();
            const auto rest = sz - prevIdx;
            for(std::size_t i = 0; i < sz; ++i)
            {
                const auto idx = (i < rest)*prevIdx + i%rest;
                assert(idx < sz);
                auto value = prevs[idx]->tryPop();
                if(!value.isNull())
                {
                    prevIdx = (idx + 1)%sz;
                    return value;
                }
            }
            return Nullable<OUT>();
        }

        template<typename OUT>
        bool OrNode<OUT>::canPop() const
        {
            assert(!prevs.empty());
            for(auto &p : prevs)
            {
                if(p->canPop())
                    return true;
            }
            return false;
        }

        template<typename OUT>
        bool OrNode<OUT>::parentsAreDone() const
        {
            for(const auto &prev : prevs)
            {
                if(!prev->parentsAreDone())
                    return false;
            }
            return true;
        }
    }
}

#endif
