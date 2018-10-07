#ifndef XPIPE_INNER_TASKNODEINLET_H
#define XPIPE_INNER_TASKNODEINLET_H

#include "xpipe/Inlet.h"

namespace xpipe
{
    namespace inner
    {
        template<typename T>
        class BaseTaskNode;

        template<typename T>
        class TaskNode;
    }

    template<typename T>
    class TaskNodeInlet: public Inlet<T>
    {
        template<class S>
        friend class inner::BaseTaskNode;
    public:
        void push(T value) override;

    protected:
        TaskNodeInlet(inner::TaskNode<T> &node)
            :node(node)
        {}

    private:
        inner::TaskNode<T> &node;
    };
}

#include "xpipe/inner/TaskNode.h"

namespace xpipe
{
    template<typename T>
    void TaskNodeInlet<T>::push(T value)
    {
        node.push(std::move(value));
    }
}

#endif
