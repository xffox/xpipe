#ifndef XPIPE_INNER_MULTIPROCTASK_H
#define XPIPE_INNER_MULTIPROCTASK_H

#include <tuple>

#include "xpipe/inner/InTypedNode.h"
#include "xpipe/inner/Task.h"
#include "xpipe/inner/StageTraits.h"
#include "xpipe/inner/MultiOutTypedNode.h"

namespace xpipe
{
    namespace inner
    {
        template<typename S>
        class MultiProcTask: public Task,
            public MultiOutStageTraits<MultiOutTypedNode, S>::TargetType,
            public InTypedNode<typename MultiOutStageTraits<MultiOutTypedNode, S>::InType>
        {
        private:
            using Parent =
                InTypedNode<
                    typename MultiOutStageTraits<MultiOutTypedNode, S>::InType>;
            using Child =
                typename MultiOutStageTraits<MultiOutTypedNode, S>::TargetType;

        public:
            MultiProcTask(S stage)
                :stage(stage)
            {}

            void init() override
            {}
            void destroy() override
            {}
            virtual bool run() override;
            virtual bool canRun() override;
            virtual bool parentsAreDone() const override;
            virtual bool childrenAreFinished() const override;

            virtual Listener *setListener(Listener *listener) override;

            virtual Task *task() override
            {
                return this;
            }

        protected:
            bool shouldFinish() const;

            void notifyPull() override
            {
                if(listener != nullptr)
                    listener->notifyPull(*this);
            }
            void notifyPush() override
            {
                if(listener != nullptr)
                    listener->notifyPush(*this);
            }
            void notifyFinished()
            {
                if(listener != nullptr)
                    listener->notifyFinished(*this);
            }

        private:
            S stage;
            bool finished = false;
            Listener *listener = nullptr;
        };

        template<typename S>
        bool MultiProcTask<S>::run()
        {
            if(finished)
            {
                MultiProcTask::notifyFinished();
                return false;
            }
            if(shouldFinish())
            {
                finished = true;
                notifyFinished();
                return false;
            }
            if(!MultiProcTask::canPush())
                return false;
            auto value = MultiProcTask::parentTryPop();
            if(!value.isNull())
            {
                if(!MultiOutStageTraits<MultiOutTypedNode, S>::TargetType::run(
                        stage, std::move(*value)))
                {
                    finished = true;
                    notifyFinished();
                    return false;
                }
                return true;
            }
            return false;
        }

        template<class S>
        bool MultiProcTask<S>::canRun()
        {
            return (MultiProcTask::canPush() &&
                MultiProcTask::parentCanPop()) || shouldFinish();
        }

        template<class S>
        bool MultiProcTask<S>::shouldFinish() const
        {
            return Parent::parentsAreDone() || Child::childrenAreFinished();
        }

        template<class S>
        bool MultiProcTask<S>::parentsAreDone() const
        {
            return finished &&
                MultiOutStageTraits<MultiOutTypedNode, S>::TargetType::queuesEmpty();
        }

        template<class S>
        bool MultiProcTask<S>::childrenAreFinished() const
        {
            return finished;
        }

        template<class S>
        typename MultiProcTask<S>::Listener *MultiProcTask<S>::setListener(Listener *listener)
        {
            auto *const prev = this->listener;
            this->listener = listener;
            return prev;
        }
    }
}

#endif
