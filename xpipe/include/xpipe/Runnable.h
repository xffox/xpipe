#ifndef XPIPE_RUNNABLE_H
#define XPIPE_RUNNABLE_H

#include "xpipe/RunnableInlet.h"

namespace xpipe
{
    template<typename T>
    class Runnable
    {
    public:
        virtual ~Runnable() = default;

        virtual void init(RunnableInlet<T> &inlet) = 0;
        virtual void destroy() = 0;
        virtual bool canRun() = 0;
        virtual bool run() = 0;
    };
}

#endif
