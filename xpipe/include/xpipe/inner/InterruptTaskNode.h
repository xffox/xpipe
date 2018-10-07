#ifndef XPIPE_INNER_INTERRUPTTASKNODE_H
#define XPIPE_INNER_INTERRUPTTASKNODE_H

#include <memory>
#include <cassert>

#include "xpipe/RunnableInlet.h"
#include "xpipe/inner/TaskNode.h"
#include "xpipe/inner/util.h"

namespace xpipe
{
    template<typename T>
    class Runnable;

    namespace inner
    {
        template<typename OUT>
        class InterruptTaskNode: public TaskNode<OUT>
        {
            using Child = TaskNode<OUT>;
        public:
            InterruptTaskNode(std::unique_ptr<Runnable<OUT>> runnable);
            ~InterruptTaskNode() override;

            void init() override;
            void destroy() override;
            bool run() override;
            bool canRun() override;
            bool parentsAreDone() const override;
            bool childrenAreFinished() const override;

            void notifyCanRun();

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

            virtual Inlet<OUT> &getInlet() override
            {
                return inlet;
            }

        private:
            RunnableInlet<OUT> inlet;
            std::unique_ptr<Runnable<OUT>> runnable;
            Node::NodeCol parents_;
            bool finished = false;
        };
    }
}

#include "xpipe/Runnable.h"

namespace xpipe
{
    namespace inner
    {
        template<typename OUT>
        InterruptTaskNode<OUT>::InterruptTaskNode(
            std::unique_ptr<Runnable<OUT>> runnable)
            :inlet(*this),
            runnable(std::move(xpipe::inner::util::notNull(runnable))),
            parents_()
        {}

        template<typename OUT>
        InterruptTaskNode<OUT>::~InterruptTaskNode()
        {}

        template<typename OUT>
        void InterruptTaskNode<OUT>::init()
        {
            assert(runnable);
            runnable->init(inlet);
        }

        template<typename OUT>
        void InterruptTaskNode<OUT>::destroy()
        {
            assert(runnable);
            runnable->destroy();
        }

        template<typename OUT>
        bool InterruptTaskNode<OUT>::run()
        {
            assert(runnable);
            if(finished)
            {
                InterruptTaskNode::notifyFinished();
                return false;
            }
            if(shouldFinish())
            {
                finished = true;
                InterruptTaskNode::notifyFinished();
                return false;
            }
            if(!InterruptTaskNode::canPush() || !runnable->canRun())
                return false;
            if(!runnable->run())
            {
                finished = true;
                InterruptTaskNode::notifyFinished();
                return false;
            }
            return true;
        }

        template<typename OUT>
        bool InterruptTaskNode<OUT>::canRun()
        {
            assert(runnable);
            return (InterruptTaskNode::canPush() && runnable->canRun()) ||
                shouldFinish();
        }

        template<typename OUT>
        bool InterruptTaskNode<OUT>::parentsAreDone() const
        {
            return finished && InterruptTaskNode::queueEmpty();
        }

        template<typename Out>
        bool InterruptTaskNode<Out>::childrenAreFinished() const
        {
            return finished;
        }

        template<typename OUT>
        void InterruptTaskNode<OUT>::notifyCanRun()
        {
            InterruptTaskNode::notifySelf();
        }

        template<typename Out>
        bool InterruptTaskNode<Out>::shouldFinish() const
        {
            return Child::childrenAreFinished();
        }
    }
}

#endif
