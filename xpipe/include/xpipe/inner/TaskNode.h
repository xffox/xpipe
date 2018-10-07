#ifndef XPIPE_INNER_TASKNODE_H
#define XPIPE_INNER_TASKNODE_H

#include "xpipe/Inlet.h"
#include "xpipe/inner/Task.h"
#include "xpipe/inner/OutTypedNode.h"
#include "xpipe/inner/AsyncQueue.h"
#include "xpipe/inner/BaseTask.h"

namespace xpipe
{
    namespace inner
    {
        template<typename OUT>
        class TaskNode: public BaseTask, public OutTypedNode<OUT>
        {
        public:
            TaskNode()
                :queue()
            {}

            Nullable<OUT> tryPop() override;
            bool canPop() const override;

            Task *task() override
            {
                return this;
            }

            void push(OUT &&value);

        protected:
            bool canPush() const;

            virtual Inlet<OUT> &getInlet() = 0;

            bool queueEmpty() const
            {
                return queue.empty();
            }

        private:
            AsyncQueue<OUT> queue;
        };

        template<typename OUT>
        Nullable<OUT> TaskNode<OUT>::tryPop()
        {
            auto value = queue.tryPop();
            if(!value.isNull())
                notifyPull();
            return value;
        }

        template<typename OUT>
        bool TaskNode<OUT>::canPop() const
        {
            return !queue.empty();
        }

        template<typename OUT>
        void TaskNode<OUT>::push(OUT &&value)
        {
            queue.push(std::move(value));
            notifyPush();
        }
        template<typename OUT>
        bool TaskNode<OUT>::canPush() const
        {
            constexpr std::size_t SIZE_LIMIT = 5;
            return queue.size() <= SIZE_LIMIT;
        }

    }
}

#endif
