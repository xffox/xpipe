#ifndef XPIPE_INNER_SINKTASKNODE_H
#define XPIPE_INNER_SINKTASKNODE_H

#include "xpipe/inner/Task.h"
#include "xpipe/inner/InTypedNode.h"

namespace xpipe
{
    namespace inner
    {
        template<class S>
        class SinkTaskNode: public BaseTask,
            public InTypedNode<typename SinkStageTraits<S>::InType>
        {
            using Parent = InTypedNode<typename SinkStageTraits<S>::InType>;
        public:
            SinkTaskNode(S stage)
                :stage(std::move(stage)), children_{}
            {}

            SinkTaskNode(const SinkTaskNode&) = delete;
            SinkTaskNode &operator=(const SinkTaskNode&) = delete;

            const Node::NodeCol &children() const override
            {
                return children_;
            }
            void clearChildren() override
            {
                children_.clear();
            }

            Task *task() override
            {
                return this;
            }

            bool run() override;
            bool canRun() override;
            bool parentsAreDone() const override;
            bool childrenAreFinished() const override
            {
                return finished;
            }

        protected:
            bool shouldFinish() const;

        private:
            S stage;
            Node::NodeCol children_;
            bool finished = false;
        };

        template<class S>
        bool SinkTaskNode<S>::run()
        {
            if(finished)
            {
                SinkTaskNode::notifyFinished();
                return false;
            }
            if(shouldFinish())
            {
                finished = true;
                SinkTaskNode::notifyFinished();
                return false;
            }
            auto value = SinkTaskNode::parentTryPop();
            if(!value.isNull())
            {
                if(!this->stage(std::move(*value)))
                {
                    finished = true;
                    SinkTaskNode::notifyFinished();
                    return false;
                }
            }
            return true;
        }

        template<class S>
        bool SinkTaskNode<S>::canRun()
        {
            return SinkTaskNode::parentCanPop() || shouldFinish();
        }

        template<class S>
        bool SinkTaskNode<S>::parentsAreDone() const
        {
            return finished;
        }

        template<class S>
        bool SinkTaskNode<S>::shouldFinish() const
        {
            return Parent::parentsAreDone();
        }
    }
}

#endif
