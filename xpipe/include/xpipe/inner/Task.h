#ifndef XPIPE_INNER_TASK_H
#define XPIPE_INNER_TASK_H

#include <vector>

namespace xpipe
{
    namespace inner
    {
        class Task
        {
        public:
            class Listener
            {
            public:
                virtual ~Listener() = default;

                virtual void notifyPush(Task &inst) = 0;
                virtual void notifyPull(Task &inst) = 0;
                virtual void notifySelf(Task &inst) = 0;
                virtual void notifyFinished(Task &inst) = 0;
            };

        public:
            virtual ~Task() = default;

            virtual void init() = 0;
            virtual void destroy() = 0;
            virtual bool run() = 0;
            virtual bool canRun() = 0;

            virtual Listener *setListener(Listener *listener) = 0;
        };
    }
}

#endif
