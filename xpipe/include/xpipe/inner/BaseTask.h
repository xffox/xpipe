#ifndef XPIPE_INNER_BASETASK_H
#define XPIPE_INNER_BASETASK_H

#include "xpipe/inner/Task.h"

namespace xpipe
{
    namespace inner
    {
        class BaseTask: public Task
        {
        public:
            void init() override
            {}
            void destroy() override
            {}

            Task::Listener *setListener(Task::Listener *listener) override
            {
                auto *oldListener = this->listener;
                this->listener = listener;
                return oldListener;
            }

        protected:
            void notifyPush()
            {
                if(listener != nullptr)
                    listener->notifyPush(*this);
            }

            void notifyPull()
            {
                if(listener != nullptr)
                    listener->notifyPull(*this);
            }

            void notifySelf()
            {
                if(listener != nullptr)
                    listener->notifySelf(*this);
            }

            void notifyFinished()
            {
                if(listener != nullptr)
                    listener->notifyFinished(*this);
            }

        private:
            Task::Listener *listener = nullptr;
        };
    }
}

#endif
