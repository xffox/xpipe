#ifndef XPIPE_INNER_STAGES_H
#define XPIPE_INNER_STAGES_H

#include <memory>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <tuple>
#include <type_traits>

#include "xpipe/Functional.h"
#include "xpipe/Runnable.h"
#include "xpipe/inner/Node.h"
#include "xpipe/inner/InTypedNode.h"
#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/ProcTaskNode.h"
#include "xpipe/inner/StageTraits.h"
#include "xpipe/inner/FromTaskNode.h"
#include "xpipe/inner/InterruptTaskNode.h"
#include "xpipe/inner/AndNode.h"
#include "xpipe/inner/SeqNode.h"
#include "xpipe/inner/OrNode.h"
#include "xpipe/inner/MultiProcTask.h"
#include "xpipe/inner/MultiOutTypedNode.h"
#include "xpipe/inner/MultiProcTask.h"
#include "xpipe/inner/InTypedNode.h"
#include "xpipe/inner/Node.h"
#include "xpipe/inner/FromTaskNode.h"
#include "xpipe/inner/OrNode.h"
#include "xpipe/inner/AndNode.h"
#include "xpipe/inner/SeqNode.h"
#include "xpipe/inner/InterruptTaskNode.h"
#include "xpipe/inner/SinkTaskNode.h"
#include "xpipe/inner/MultiOutConsumerNode.h"
#include "xpipe/inner/ParNode.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    class Pipeline;

    class BaseStage
    {
    public:
        explicit BaseStage(inner::graphptr::NodePointer<inner::Node> node)
            :node(node)
        {}
        virtual ~BaseStage() = default;

        inner::graphptr::NodePointer<inner::Node> getNode() const
        {
            return node;
        }

    private:
        inner::graphptr::NodePointer<inner::Node> node;
    };

    template<typename Out, typename... Outs>
    class OutStage: public virtual BaseStage
    {
        using NodeTuple =
            std::tuple<inner::graphptr::NodePointer<inner::OutTypedNode<Out>>,
                inner::graphptr::NodePointer<inner::OutTypedNode<Outs>>...>;
    public:
        explicit OutStage(const NodeTuple &outTasks)
            :BaseStage(std::get<0>(outTasks)), outTasks(outTasks)
        {}

        template<std::size_t I>
        OutStage<typename std::tuple_element<I, std::tuple<Out, Outs...>>::type>
        get() const
        {
            return OutStage<typename std::tuple_element<I, std::tuple<Out, Outs...>>::type>(
                getOutTask<I>());
        }

        template<std::size_t I>
        inner::graphptr::NodePointer<inner::OutTypedNode<
            typename std::tuple_element<I, std::tuple<Out, Outs...>>::type>>
        getOutTask() const
        {
            return std::get<I>(outTasks);
        }

        const NodeTuple &getOutTasks() const
        {
            return outTasks;
        }

    private:
        NodeTuple outTasks;
    };

    template<typename In>
    class InStage: public virtual BaseStage
    {
    public:
        explicit InStage(inner::graphptr::NodePointer<inner::InTypedNode<In>> node)
            :BaseStage(node), inNode(node)
        {}

        inner::graphptr::NodePointer<inner::InTypedNode<In>> getInTask() const
        {
            return inNode;
        }

    private:
        inner::graphptr::NodePointer<inner::InTypedNode<In>> inNode;
    };

    template<typename IN, typename Out, typename... Outs>
    class Stage: public InStage<IN>, public OutStage<Out, Outs...>
    {
        using NodeTuple =
            std::tuple<inner::graphptr::NodePointer<inner::OutTypedNode<Out>>,
                inner::graphptr::NodePointer<inner::OutTypedNode<Outs>>...>;
    public:
        Stage(
            const NodeTuple &childTasks,
            inner::graphptr::NodePointer<inner::InTypedNode<IN>> parentTask)
            :BaseStage(parentTask),
            InStage<IN>(parentTask), OutStage<Out, Outs...>(childTasks)
        {}

        template<std::size_t I>
        Stage<IN, typename std::tuple_element<I, std::tuple<Out, Outs...>>::type>
        get() const
        {
            return Stage<IN, typename std::tuple_element<I, std::tuple<Out, Outs...>>::type>(
                OutStage<Out, Outs...>::template getOutTask<I>(), Stage::getInTask());
        }
    };

    namespace inner
    {
        template<typename In, typename Out>
        void linkNodes(
            graphptr::NodePointer<inner::OutTypedNode<Out>> parent,
            graphptr::NodePointer<inner::InTypedNode<In>> child)
        {
            for(auto parentChild : parent->children())
            {
                parentChild->clearParents();
            }
            for(auto childParent : child->parents())
            {
                childParent->clearChildren();
            }
            parent->setChild(parent.link(child));
            child->setParent(child.link(parent));
        }

        template<typename In, typename... Outs, std::size_t... Is>
        Stage<In, Outs...> makeMultiConsumers(
            graphptr::NodePointer<inner::InTypedNode<In>> inNode,
            graphptr::NodePointer<inner::MultiOutTypedNode<Outs...>> outNode,
            IndexSequence<Is...>)
        {
            auto children = std::make_tuple(
                graphptr::make_node<
                    inner::MultiOutConsumerNode<Is, Outs...>>()...);
            outNode->setChildren(std::make_tuple(
                    outNode.link(std::get<Is>(children))...));
            Pass{(std::get<Is>(children)->setParent(
                    std::get<Is>(children).link(outNode)),nullptr)...};
            return Stage<In, Outs...>(children, inNode);
        }
        template<typename In, typename... Outs>
        Stage<In, Outs...> makeMultiConsumers(
            graphptr::NodePointer<inner::InTypedNode<In>> inNode,
            graphptr::NodePointer<inner::MultiOutTypedNode<Outs...>> node)
        {
            return makeMultiConsumers(inNode, node,
                MakeIndexSequence<sizeof...(Outs)>());
        }

        template<class Out, class... Outs, std::size_t... Is>
        OutStage<Out> anyFrom(const OutStage<Out, Outs...> &stage,
            IndexSequence<Is...>)
        {
            auto child = inner::graphptr::make_node<inner::OrNode<Out>>();
            auto parents = typename inner::OrNode<Out>::NodePtrCol{
                child.link(stage.template getOutTask<Is>())...};
            child->setParents(parents);
            Pass{(stage.template getOutTask<Is>()->setChild(
                    stage.template getOutTask<Is>().link(child)),nullptr)...};
            return OutStage<Out>(child);
        }
    }

    template<class S>
    OutStage<typename inner::SourceStageTraits<S>::OutType> source(S &&stage)
    {
        return OutStage<typename inner::SourceStageTraits<S>::OutType>(
            inner::graphptr::make_node<inner::FromTaskNode<S>>(
                std::forward<S>(stage)));
    }

    template<class S>
    InStage<typename inner::SinkStageTraits<S>::InType> sink(S &&stage)
    {
        return InStage<typename inner::SinkStageTraits<S>::InType>(
            inner::graphptr::make_node<inner::SinkTaskNode<S>>(
                std::forward<S>(stage)));
    }

    template<class S>
    Stage<typename inner::StageTraits<S>::InType,
        typename inner::StageTraits<S>::OutType> map(S &&stage)
    {
        using DS = typename std::decay<S>::type;
        auto node = inner::graphptr::make_node<inner::ProcTaskNode<DS>>(
            std::forward<S>(stage));
        return Stage<typename inner::StageTraits<S>::InType,
               typename inner::StageTraits<S>::OutType>(
                   node, node);
    }

    template<typename Out, typename... Outs>
    OutStage<Out> any(
        const OutStage<Out> &stage,
        const OutStage<Outs>&... stages)
    {
        auto child = inner::graphptr::make_node<inner::OrNode<Out>>();
        auto parents = typename inner::OrNode<Out>::NodePtrCol{
            child.link(stage.template getOutTask<0>()),
            child.link(stages.template getOutTask<0>())...};
        child->setParents(parents);
        Pass{(stage.template getOutTask<0>()->setChild(
                stage.template getOutTask<0>().link(child)),nullptr),
            (stages.template getOutTask<0>()->setChild(
                stages.template getOutTask<0>().link(child)),nullptr)...
        };
        return OutStage<Out>(child);
    }

    template<class In, class Out, class... Outs>
    Stage<In, Out> anyFrom(const Stage<In, Out, Outs...> &stage)
    {
        auto outStage =
            inner::anyFrom(stage, MakeIndexSequence<sizeof...(Outs)+1>());
        return Stage<In, Out>(
            outStage.template getOutTask<0>(), stage.getInTask());
    }

    template<class Out, class... Outs>
    OutStage<Out> anyFrom(const OutStage<Out, Outs...> &stage)
    {
        return inner::anyFrom(stage, MakeIndexSequence<sizeof...(Outs)+1>());
    }

    template<class Out, class... Outs>
    OutStage<std::tuple<Out, Outs...>> all(
        const OutStage<Out> &stage,
        const OutStage<Outs>&... stages)
    {
        auto child = inner::graphptr::make_node<inner::AndNode<Out, Outs...>>();
        auto parents = std::make_tuple(
            child.link(stage.template getOutTask<0>()),
            child.link(stages.template getOutTask<0>())...);
        child->setParents(parents);
        Pass{(stage.template getOutTask<0>()->setChild(
                stage.template getOutTask<0>().link(child)),nullptr),
            (stages.template getOutTask<0>()->setChild(
                stages.template getOutTask<0>().link(child)),nullptr)...
        };
        return OutStage<std::tuple<Out, Outs...>>(child);
    }

    template<class Out, class... Outs>
    OutStage<Out> seq(
        const OutStage<Out> &stage,
        const OutStage<Outs>&... stages)
    {
        auto child = inner::graphptr::make_node<inner::SeqNode<Out>>();
        typename inner::SeqNode<Out>::NodePtrCol parents{
            child.link(stage.template getOutTask<0>()),
            child.link(stages.template getOutTask<0>())...};
        child->setParents(parents);
        Pass{(stage.template getOutTask<0>()->setChild(
                stage.template getOutTask<0>().link(child)),nullptr),
            (stages.template getOutTask<0>()->setChild(
                stages.template getOutTask<0>().link(child)),nullptr)...
        };
        return OutStage<Out>(child);
    }

    template<class Out>
    OutStage<Out> use(std::unique_ptr<Runnable<Out>> runnable)
    {
        return OutStage<Out>(
            inner::graphptr::make_node<inner::InterruptTaskNode<Out>>(
                std::move(runnable)));
    }

    template<typename IN, typename MID, typename... OUT>
    Stage<IN, OUT...> combine(const Stage<IN, MID> &parentStage,
        const Stage<MID, OUT...> &childStage)
    {
        linkNodes(parentStage.template getOutTask<0>(),
            childStage.getInTask());
        return Stage<IN, OUT...>(
            childStage.getOutTasks(),
            parentStage.template getInTask());
    }

    template<typename MID, typename... OUT>
    OutStage<OUT...> combine(const OutStage<MID> &parentStage,
        const Stage<MID, OUT...> &childStage)
    {
        linkNodes(parentStage.template getOutTask<0>(),
            childStage.getInTask());
        return childStage;
    }

    template<typename In, typename Mid>
    InStage<In> combine(const Stage<In, Mid> &parentStage,
        const InStage<Mid> &childStage)
    {
        linkNodes(parentStage.template getOutTask<0>(),
            childStage.getInTask());
        return parentStage;
    }

    template<typename In>
    BaseStage combine(const OutStage<In> &parentStage,
        const InStage<In> &childStage)
    {
        linkNodes(parentStage.template getOutTask<0>(),
            childStage.getInTask());
        return parentStage.template get<0>();
    }

    template<class S>
    typename inner::MultiOutStageTraits<Stage, S>::FuncType multimap(S stage)
    {
        auto node = inner::graphptr::make_node<inner::MultiProcTask<S>>(stage);
        return makeMultiConsumers(
            inner::graphptr::NodePointer<inner::InTypedNode<
                typename inner::MultiOutStageTraits<
                    inner::MultiOutTypedNode, S>::InType>>(node),
            inner::graphptr::NodePointer<
                typename inner::MultiOutStageTraits<
                    inner::MultiOutTypedNode, S>::TargetType>(node));
    }

    template<typename In, typename... Outs>
    Stage<In, Outs...> parmap(const Stage<In, Outs>&... stages)
    {
        auto parent = inner::graphptr::make_node<inner::ParNode<In>>();
        typename inner::ParNode<In>::NodePtrCol children{
            parent.link(stages.template getOutTask<0>())...
        };
        parent->setChildren(children);
        Pass{(stages.getInTask()->setParent(
                stages.template getOutTask<0>().link(parent)),nullptr)...};
        return Stage<In, Outs...>(
            std::make_tuple(stages.template getOutTask<0>()...),
            parent);
    }
}

template<class OUT>
xpipe::OutStage<OUT> operator||(
    const xpipe::OutStage<OUT> &left,
    const xpipe::OutStage<OUT> &right)
{
    return xpipe::any(left, right);
}

template<class OUTL, class OUTR>
xpipe::OutStage<std::tuple<OUTL, OUTR>> operator&&(
    const xpipe::OutStage<OUTL> &left,
    const xpipe::OutStage<OUTR> &right)
{
    return xpipe::all(left, right);
}

template<class IN, class MID, class... OUT>
xpipe::Stage<IN, OUT...> operator>>(
    xpipe::Stage<IN, MID> parentStage,
    xpipe::Stage<MID, OUT...> childStage)
{
    return xpipe::combine(parentStage, childStage);
}

template<class MID, class... OUT>
xpipe::OutStage<OUT...> operator>>(
    xpipe::OutStage<MID> parentStage,
    xpipe::Stage<MID, OUT...> childStage)
{
    return xpipe::combine(parentStage, childStage);
}

template<class In, class Mid>
xpipe::InStage<In> operator>>(
    xpipe::Stage<In, Mid> parentStage,
    xpipe::InStage<Mid> childStage)
{
    return xpipe::combine(parentStage, childStage);
}

template<class In>
xpipe::BaseStage operator>>(
    xpipe::OutStage<In> parentStage,
    xpipe::InStage<In> childStage)
{
    return xpipe::combine(parentStage, childStage);
}

#endif
