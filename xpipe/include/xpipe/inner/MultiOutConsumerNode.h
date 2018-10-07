#ifndef XPIPE_INNER_MULTIOUTCONSUMERNODE_H
#define XPIPE_INNER_MULTIOUTCONSUMERNODE_H

#include <cstddef>
#include <memory>
#include <cassert>

#include "xpipe/inner/MultiOutTypedNode.h"
#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<std::size_t I, class... Args>
        class MultiOutConsumerNode:
            public OutTypedNode<typename std::tuple_element<I, std::tuple<Args...>>::type>
        {
        public:
            MultiOutConsumerNode()
                :prev(), parents_{}
            {}

            MultiOutConsumerNode(const MultiOutConsumerNode&) = delete;
            MultiOutConsumerNode &operator=(
                const MultiOutConsumerNode&) = delete;

            void setParent(
                graphptr::LinkPointer<MultiOutTypedNode<Args...>> prev);

            Task *task() override
            {
                return nullptr;
            }

            Nullable<typename std::tuple_element<I, std::tuple<Args...>>::type>
                tryPop() override;
            bool canPop() const override;
            bool parentsAreDone() const override;

            const Node::NodeCol &parents() const override
            {
                return parents_;
            }
            void clearParents() override
            {
                parents_.clear();
                prev = graphptr::LinkPointer<MultiOutTypedNode<Args...>>();
            }

        private:
            graphptr::LinkPointer<MultiOutTypedNode<Args...>> prev;
            Node::NodeCol parents_;
        };

        template<std::size_t I, class... Args>
        void MultiOutConsumerNode<I, Args...>::setParent(
            graphptr::LinkPointer<MultiOutTypedNode<Args...>> prev)
        {
            this->prev = prev;
            parents_ = Node::NodeCol{prev.get()};
        }

        template<std::size_t I, class... Args>
        Nullable<typename std::tuple_element<I, std::tuple<Args...>>::type> MultiOutConsumerNode<I, Args...>::tryPop()
        {
            assert(prev);
            return prev->template tryPop<I>();
        }

        template<std::size_t I, class... Args>
        bool MultiOutConsumerNode<I, Args...>::canPop() const
        {
            assert(prev);
            return prev->template canPop<I>();
        }

        template<std::size_t I, class... Args>
        bool MultiOutConsumerNode<I, Args...>::parentsAreDone() const
        {
            assert(prev);
            return prev->parentsAreDone();
        }
    }
}

#endif
