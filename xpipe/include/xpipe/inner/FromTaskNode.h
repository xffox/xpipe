#ifndef XPIPE_INNER_FROMTASKNODE_H
#define XPIPE_INNER_FROMTASKNODE_H

#include "xpipe/inner/StageTraits.h"
#include "xpipe/inner/BaseTaskNode.h"

namespace xpipe
{
    namespace inner
    {
        template<class S>
        class FromTaskNode:
            public BaseTaskNode<typename SourceStageTraits<S>::OutType>
        {
            using Child = BaseTaskNode<typename SourceStageTraits<S>::OutType>;
        public:
            FromTaskNode(S stage)
                :stage(std::move(stage)), parents_()
            {}

            FromTaskNode(const FromTaskNode&) = delete;
            FromTaskNode &operator=(const FromTaskNode&) = delete;

            Task *task() override
            {
                return this;
            }

            bool run() override;
            bool canRun() override;
            bool parentsAreDone() const override;
            bool childrenAreFinished() const override;

            const Node::NodeCol &parents() const override
            {
                return parents_;
            }
            void clearParents() override
            {
                parents_.clear();
            }

        protected:
            bool shouldFinish() const;

        private:
            S stage;
            Node::NodeCol parents_;
            bool finished = false;
        };

        template<class S>
        bool FromTaskNode<S>::run()
        {
            if(finished)
            {
                FromTaskNode::notifyFinished();
                return false;
            }
            if(shouldFinish())
            {
                finished = true;
                FromTaskNode::notifyFinished();
                return false;
            }
            if(!FromTaskNode::canPush())
                return false;
            if(!this->stage(FromTaskNode::getInlet()))
            {
                finished = true;
                FromTaskNode::notifyFinished();
                return false;
            }
            return true;
        }

        template<class S>
        bool FromTaskNode<S>::canRun()
        {
            return FromTaskNode::canPush() || shouldFinish();
        }

        template<class S>
        bool FromTaskNode<S>::parentsAreDone() const
        {
            return finished && FromTaskNode::queueEmpty();
        }

        template<class S>
        bool FromTaskNode<S>::childrenAreFinished() const
        {
            return finished;
        }

        template<class S>
        bool FromTaskNode<S>::shouldFinish() const
        {
            return Child::childrenAreFinished();
        }
    }
}

#endif
