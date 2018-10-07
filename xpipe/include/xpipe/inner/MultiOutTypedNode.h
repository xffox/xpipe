#ifndef XPIPE_INNER_MULTIOUTTYPEDNODE_H
#define XPIPE_INNER_MULTIOUTTYPEDNODE_H

#include <cstddef>
#include <tuple>
#include <cstddef>

#include "xpipe/Functional.h"
#include "xpipe/IndexSequence.h"
#include "xpipe/Inlet.h"
#include "xpipe/inner/Node.h"
#include "xpipe/inner/Nullable.h"
#include "xpipe/inner/StageTraits.h"
#include "xpipe/inner/AsyncQueue.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<typename... Outs>
        class MultiOutTypedNode: public virtual Node
        {
        public:
            using TaskTuple =
                std::tuple<graphptr::LinkPointer<OutTypedNode<Outs>>...>;
        public:
            template<std::size_t I>
            Nullable<typename std::tuple_element<I, std::tuple<Outs...>>::type>
                tryPop();
            template<std::size_t I>
            bool canPop() const;

            const NodeCol &children() const override
            {
                return children_;
            }
            void clearChildren() override
            {
                children_.clear();
                childrenTasks = TaskTuple();
            }
            bool childrenAreFinished() const override;

            void setChildren(const TaskTuple &children)
            {
                childrenTasks = children;
                children_ = makeNodes(children,
                    MakeIndexSequence<sizeof...(Outs)>());
            }

        protected:
            virtual void notifyPull() = 0;
            virtual void notifyPush() = 0;

            template<class S>
            bool run(S &&stage, typename MultiOutStageTraits<MultiOutTypedNode, S>::InType &&value);
            bool canPush() const;
            bool queuesEmpty() const;

        private:
            template<std::size_t I>
            class MultiInlet:
                public Inlet<
                typename std::tuple_element<I, std::tuple<Outs...>>::type>
            {
            public:
                void push(
                    typename std::tuple_element<I, std::tuple<Outs...>>::type value) override
                {
                    node.template push<I>(value);
                }

                MultiInlet(MultiOutTypedNode<Outs...> &node)
                    :node(node)
                {}

            private:
                MultiOutTypedNode<Outs...> &node;
            };

        private:
            template<std::size_t I>
            void push(
                typename std::tuple_element<I, std::tuple<Outs...>>::type value)
            {
                std::get<I>(queues).push(std::move(value));
                notifyPush();
            }

            template<std::size_t... I>
            bool canPush(IndexSequence<I...>) const;
            template<std::size_t... I>
            bool queuesEmpty(IndexSequence<I...>) const;
            template<class S, std::size_t... I>
            bool innerRun(S &stage, typename MultiOutStageTraits<MultiOutTypedNode, S>::InType &&value,
                IndexSequence<I...>);

            template<std::size_t... I>
            static Node::NodeCol makeNodes(
                const TaskTuple &prevs, IndexSequence<I...>)
            {
                return Node::NodeCol{std::get<I>(prevs).get()...};
            }

        private:
            std::tuple<AsyncQueue<Outs>...> queues;
            TaskTuple childrenTasks;
            NodeCol children_;
        };

        template<typename... Outs>
        template<std::size_t I>
        Nullable<typename std::tuple_element<I, std::tuple<Outs...>>::type>
            MultiOutTypedNode<Outs...>::tryPop()
        {
            auto value = std::get<I>(queues).tryPop();
            if(!value.isNull())
                notifyPull();
            return value;
        }

        template<typename... Outs>
        template<std::size_t I>
        bool MultiOutTypedNode<Outs...>::canPop() const
        {
            return !std::get<I>(queues).empty();
        }

        template<typename... Outs>
        template<class S>
        bool MultiOutTypedNode<Outs...>::run(S &&stage,
            typename MultiOutStageTraits<MultiOutTypedNode, S>::InType &&value)
        {
            return innerRun(std::forward<S>(stage), std::move(value),
                MakeIndexSequence<sizeof...(Outs)>());
        }

        template<typename... Outs>
        bool MultiOutTypedNode<Outs...>::canPush() const
        {
            return canPush(MakeIndexSequence<sizeof...(Outs)>());
        }

        template<typename... Outs>
        bool MultiOutTypedNode<Outs...>::queuesEmpty() const
        {
            return queuesEmpty(MakeIndexSequence<sizeof...(Outs)>());
        }

        template<typename... Outs>
        template<std::size_t... I>
        bool MultiOutTypedNode<Outs...>::canPush(IndexSequence<I...>) const
        {
            // TODO: move to proper place
            constexpr std::size_t SIZE_LIMIT = 5;
            return reduce([](bool l, bool r){return l && r;}, true,
                (std::get<I>(queues).size() <= SIZE_LIMIT)...);
        }

        template<typename... Outs>
        template<std::size_t... I>
        bool MultiOutTypedNode<Outs...>::queuesEmpty(IndexSequence<I...>) const
        {
            return reduce([](bool l, bool r){return l && r;}, true,
                std::get<I>(queues).empty()...);
        }

        template<typename... Outs>
        template<class S, std::size_t... I>
        bool MultiOutTypedNode<Outs...>::innerRun(
            S &stage, typename MultiOutStageTraits<MultiOutTypedNode, S>::InType &&value,
            IndexSequence<I...>)
        {
            auto inlets = std::make_tuple(MultiInlet<I>(*this)...);
            return stage(std::move(value), std::get<I>(inlets)...);
        }

        template<typename... Out>
        bool MultiOutTypedNode<Out...>::childrenAreFinished() const
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
