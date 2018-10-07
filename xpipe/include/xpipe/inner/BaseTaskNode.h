#ifndef XPIPE_INNER_BASETASKNODE_H
#define XPIPE_INNER_BASETASKNODE_H

#include "xpipe/TaskNodeInlet.h"
#include "xpipe/inner/TaskNode.h"

namespace xpipe
{
    namespace inner
    {
        template<typename OUT>
        class BaseTaskNode: public TaskNode<OUT>
        {
        public:
            BaseTaskNode()
                :inlet(*this)
            {}

        protected:
            virtual Inlet<OUT> &getInlet() override
            {
                return inlet;
            }

        private:
            TaskNodeInlet<OUT> inlet;
        };
    }
}

#endif
