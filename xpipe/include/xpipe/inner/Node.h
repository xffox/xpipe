#ifndef XPIPE_INNER_NODE_H
#define XPIPE_INNER_NODE_H

#include <vector>

namespace xpipe
{
    namespace inner
    {
        class Task;

        class Node
        {
        public:
            using NodeCol = std::vector<Node*>;

        public:
            virtual ~Node() = default;

            virtual const NodeCol &parents() const = 0;
            virtual const NodeCol &children() const = 0;
            virtual void clearParents() = 0;
            virtual void clearChildren() = 0;
            virtual Task *task() = 0;
            virtual bool parentsAreDone() const = 0;
            virtual bool childrenAreFinished() const = 0;
        };
    }
}

#endif
