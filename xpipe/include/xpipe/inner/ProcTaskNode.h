#ifndef XPIPE_INNER_PROCTASKNODE_H
#define XPIPE_INNER_PROCTASKNODE_H

#include <memory>
#include <cassert>
#include <vector>

#include "xpipe/inner/BaseTaskNode.h"
#include "xpipe/inner/StageTraits.h"
#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/InTypedNode.h"

namespace xpipe
{
    namespace inner
    {
        template<class S>
        class ProcTaskNode:
            public BaseTaskNode<typename StageTraits<S>::OutType>,
            public InTypedNode<typename StageTraits<S>::InType>
        {
        private:
            using Parent = InTypedNode<typename StageTraits<S>::InType>;
            using Child = BaseTaskNode<typename StageTraits<S>::OutType>;

        public:
            ProcTaskNode(S stage);

            ProcTaskNode(const ProcTaskNode&) = delete;
            ProcTaskNode &operator=(const ProcTaskNode&) = delete;

            bool run() override;
            bool canRun() override;
            bool parentsAreDone() const override;
            bool childrenAreFinished() const override;

        protected:
            bool shouldFinish() const;

        private:
            S stage;
            Node::NodeCol parents_;
            volatile bool finished = false;
        };

        template<class S>
        ProcTaskNode<S>::ProcTaskNode(S stage)
            :stage(std::move(stage))
        {}

        template<class S>
        bool ProcTaskNode<S>::run()
        {
            if(finished)
            {
                ProcTaskNode::notifyFinished();
                return false;
            }
            if(shouldFinish())
            {
                finished = true;
                ProcTaskNode::notifyFinished();
                return false;
            }
            if(!ProcTaskNode::canPush())
                return false;
            auto value = ProcTaskNode::parentTryPop();
            if(!value.isNull())
            {
                if(!this->stage(std::move(*value), ProcTaskNode::getInlet()))
                {
                    finished = true;
                    ProcTaskNode::notifyFinished();
                    return false;
                }
                return true;
            }
            return false;
        }

        template<class S>
        bool ProcTaskNode<S>::canRun()
        {
            return (ProcTaskNode::canPush() && ProcTaskNode::parentCanPop()) ||
                shouldFinish();
        }

        template<class S>
        bool ProcTaskNode<S>::shouldFinish() const
        {
            return Parent::parentsAreDone() || Child::childrenAreFinished();
        }

        template<class S>
        bool ProcTaskNode<S>::parentsAreDone() const
        {
            return finished &&
                ProcTaskNode::queueEmpty();
        }

        template<class S>
        bool ProcTaskNode<S>::childrenAreFinished() const
        {
            return finished;
        }
    }
}

#endif
