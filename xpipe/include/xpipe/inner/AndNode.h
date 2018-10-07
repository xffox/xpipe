#ifndef XPIPE_INNER_TYPEDANDTASK_H
#define XPIPE_INNER_TYPEDANDTASK_H

#include <memory>
#include <cassert>
#include <tuple>
#include <utility>

#include "xpipe/Functional.h"
#include "xpipe/IndexSequence.h"
#include "xpipe/inner/StageTraits.h"
#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    namespace inner
    {
        template<typename... Args>
        class AndNode: public OutTypedNode<std::tuple<Args...>>
        {
        public:
            using TaskTuple =
                std::tuple<graphptr::LinkPointer<OutTypedNode<Args>>...>;
            using OutType = std::tuple<Args...>;

        public:
            AndNode();

            AndNode(const AndNode&) = delete;
            AndNode &operator=(const AndNode&) = delete;

            void setParents(const TaskTuple &prevs);

            Task *task()
            {
                return nullptr;
            }

            Nullable<OutType> tryPop() override;
            bool canPop() const override;
            bool parentsAreDone() const override;

            const Node::NodeCol &parents() const override
            {
                return parents_;
            }
            void clearParents() override
            {
                parents_.clear();
                prevs = TaskTuple();
            }

        private:
            template<std::size_t... I>
            std::tuple<Args...> pop(IndexSequence<I...>);

            template<std::size_t... I>
            bool isDone(IndexSequence<I...>) const;

            template<std::size_t... I>
            bool canPop(IndexSequence<I...>) const;

            template<std::size_t... I>
            static Node::NodeCol makeParents(const TaskTuple &prevs,
                IndexSequence<I...>);

        private:
            TaskTuple prevs;
            Node::NodeCol parents_;
        };


        template<typename... Args>
        AndNode<Args...>::AndNode()
            :prevs(), parents_()
        {}

        template<typename... Args>
        void AndNode<Args...>::setParents(const TaskTuple &parents)
        {
            this->prevs = parents;
            this->parents_ = makeParents(parents,
                MakeIndexSequence<sizeof...(Args)>());
        }

        template<typename... Args>
        bool AndNode<Args...>::canPop() const
        {
            return canPop(MakeIndexSequence<sizeof...(Args)>());
        }

        template<typename... Args>
        Nullable<typename AndNode<Args...>::OutType> AndNode<Args...>::tryPop()
        {
            if(!canPop())
                return Nullable<OutType>();
            return Nullable<OutType>(pop(
                    MakeIndexSequence<sizeof...(Args)>()));
        }

        template<typename... Args>
        bool AndNode<Args...>::parentsAreDone() const
        {
            return isDone(MakeIndexSequence<sizeof...(Args)>());
        }

        template<typename... Args>
        template<std::size_t... I>
        std::tuple<Args...> AndNode<Args...>::pop(IndexSequence<I...>)
        {
            return std::make_tuple(
                std::move(*std::get<I>(prevs)->tryPop())...);
        }

        template<typename... Args>
        template<std::size_t... I>
        bool AndNode<Args...>::isDone(IndexSequence<I...>) const
        {
            return reduce([](bool l, bool r){return l || r;}, false,
                std::get<I>(prevs)->parentsAreDone()...);
        }

        template<typename... Args>
        template<std::size_t... I>
        bool AndNode<Args...>::canPop(IndexSequence<I...>) const
        {
            return reduce([](bool l, bool r){return l && r;}, true,
                std::get<I>(prevs)->canPop()...);
        }

        template<typename... Args>
        template<std::size_t... I>
        Node::NodeCol AndNode<Args...>::makeParents(const TaskTuple &prevs,
            IndexSequence<I...>)
        {
            return Node::NodeCol{std::get<I>(prevs).get()...};
        }
    }
}

#endif
