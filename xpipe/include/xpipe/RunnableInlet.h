#ifndef XPIPE_RUNNABLEINLET_H
#define XPIPE_RUNNABLEINLET_H

#include "xpipe/TaskNodeInlet.h"

namespace xpipe
{
    namespace inner
    {
        template<typename T>
        class InterruptTaskNode;
    }

    template<typename T>
    class RunnableInlet: public TaskNodeInlet<T>
    {
        template<class S>
        friend class inner::InterruptTaskNode;
    public:
        void notifyCanRun();

    private:
        RunnableInlet(inner::InterruptTaskNode<T> &node)
            :TaskNodeInlet<T>(node), node(node)
        {}

    private:
        inner::InterruptTaskNode<T> &node;
    };
}

#include "xpipe/inner/InterruptTaskNode.h"

namespace xpipe
{
    template<typename T>
    void RunnableInlet<T>::notifyCanRun()
    {
        node.notifyCanRun();
    }
}

#endif
