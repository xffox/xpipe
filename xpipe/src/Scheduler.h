#ifndef XPIPE_SCHEDULER_H
#define XPIPE_SCHEDULER_H

#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include <queue>
#include <vector>

#include "xpipe/inner/Node.h"
#include "xpipe/inner/Task.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    class Scheduler: public inner::Task::Listener
    {
    public:
        Scheduler(inner::graphptr::NodePointer<inner::Node> node);
        ~Scheduler() override;

        void start();
        void stop();

        inner::Task *takeTask();
        void putTask(inner::Task *task);

    protected:
        void notifyPush(inner::Task &inst) override;
        void notifyPull(inner::Task &inst) override;
        void notifySelf(inner::Task &inst) override;
        void notifyFinished(inner::Task &inst) override;

    private:
        struct PrioritizedTask
        {
            inner::Task *task;
            std::size_t priority;
        };
        struct PrioritizedTaskLess
        {
            bool operator()(const PrioritizedTask &left,
                const PrioritizedTask &right) const
            {
                return left.priority < right.priority;
            }
        };

        using TaskSet = std::unordered_set<inner::Task*>;
        using NodeSet = std::unordered_set<inner::Node*>;
        using TaskPriorityMap = std::unordered_map<inner::Task*, std::size_t>;
        using TaskQueue = std::priority_queue<PrioritizedTask,
              std::vector<PrioritizedTask>, PrioritizedTaskLess>;
        using TaskDepMap = std::unordered_map<inner::Task*, TaskSet>;

    private:
        void updateReadiness(std::lock_guard<std::mutex> &lock,
            inner::Task &task);
        void updateChildrenReadiness(std::lock_guard<std::mutex> &lock,
            inner::Task &task);
        void updateParentsReadiness(std::lock_guard<std::mutex> &lock,
            inner::Task &task);
        void markReady(std::lock_guard<std::mutex>&, inner::Task &task);
        void markFinished(std::lock_guard<std::mutex>&, inner::Task &task);

        template<typename Expand>
        static TaskSet findTasks(inner::Node &node, Expand expand);
        static TaskSet findParentTasks(inner::Node &node);
        static TaskSet findChildTasks(inner::Node &node);

    private:
        inner::graphptr::NodePointer<inner::Node> node;
        std::mutex mutex;
        std::condition_variable cond;
        bool cont = true;
        TaskQueue ready;
        TaskSet waiting;
        TaskSet unfinishedTasks;
        TaskDepMap childDeps;
        TaskDepMap parentDeps;
        TaskPriorityMap priorities;
        TaskSet tasks;
    };
}

#endif
