#include "Scheduler.h"

#include <stdexcept>
#include <utility>
#include <queue>
#include <cassert>
#include <unordered_set>
#include <limits>
#include <iterator>

namespace xpipe
{
    namespace
    {
        using NodeSet = std::unordered_set<inner::Node*>;
        using NodeQueue = std::queue<inner::Node*>;

        NodeSet findRoots(inner::Node &node)
        {
            NodeSet roots;
            NodeSet seenNodes;
            NodeQueue front;
            front.push(&node);
            seenNodes.insert(&node);
            inner::Node *lastNode = nullptr;
            while(!front.empty())
            {
                auto *cur = front.front();
                front.pop();
                assert(cur);
                const auto &children = cur->children();
                for(auto *c : children)
                {
                    if(seenNodes.insert(c).second)
                    {
                        front.push(c);
                    }
                }
                const auto &parents = cur->parents();
                for(auto *p : parents)
                {
                    if(seenNodes.insert(p).second)
                    {
                        front.push(p);
                    }
                }
                if(parents.empty())
                {
                    roots.insert(cur);
                }
                lastNode = cur;
            }
            if(roots.empty() && lastNode != nullptr)
                roots.insert(lastNode);
            return roots;
        }
    }

    Scheduler::Scheduler(inner::graphptr::NodePointer<inner::Node> node)
        :node(node), mutex(), cond(),
        ready(), waiting(),
        childDeps(), parentDeps(), priorities(), tasks()
    {
        auto roots = findRoots(*node);
        using CandQueue = std::queue<inner::Node*>;
        CandQueue front;
        NodeSet seenNodes;
        for(auto *n : roots)
        {
            front.push(n);
            seenNodes.insert(n);
        }
        std::size_t priority = std::numeric_limits<std::size_t>::max();
        while(!front.empty())
        {
            auto *cur = front.front();
            front.pop();
            assert(cur);
            auto *const curTask = cur->task();
            if(curTask)
            {
                tasks.insert(curTask);
                const auto r = priorities.insert(
                    std::make_pair(curTask, priority--)).second;
                (void)r;
                assert(r);
                unfinishedTasks.insert(curTask);
                waiting.insert(curTask);
                const auto childDepInsertRes = childDeps.insert(
                    std::make_pair(curTask, findChildTasks(*cur))).second;
                (void)childDepInsertRes;
                assert(childDepInsertRes);
                const auto parentDepInsertRes = parentDeps.insert(
                    std::make_pair(curTask, findParentTasks(*cur))).second;
                (void)parentDepInsertRes;
                assert(parentDepInsertRes);
                curTask->setListener(this);
            }
            const auto &children = cur->children();
            for(auto *c : children)
            {
                assert(c);
                if(seenNodes.insert(c).second)
                {
                    front.push(c);
                }
            }
        }
    }

    Scheduler::~Scheduler()
    {
        for(auto *task : tasks)
        {
            assert(task);
            task->setListener(nullptr);
        }
    }

    void Scheduler::start()
    {
        std::lock_guard<std::mutex> lock(mutex);
        for(auto iter = begin(waiting); iter != end(waiting);)
        {
            if((*iter)->canRun())
            {
                auto *readyTask = *iter;
                waiting.erase(iter++);
                markReady(lock, *readyTask);
            }
            else
            {
                ++iter;
            }
        }
    }

    void Scheduler::stop()
    {
        std::lock_guard<std::mutex> lock(mutex);
        cont = false;
        cond.notify_all();
    }

    inner::Task *Scheduler::takeTask()
    {
        std::unique_lock<std::mutex> lock(mutex);
        while(true)
        {
            if(cont)
            {
                if(ready.empty())
                {
                    if(unfinishedTasks.empty())
                        return nullptr;
                    cond.wait(lock, [this](){
                            return !cont || !ready.empty() ||
                                unfinishedTasks.empty();
                        });
                }
                else
                {
                    const auto task = ready.top();
                    ready.pop();
                    return task.task;
                }
            }
            else
            {
                return nullptr;
            }
        }
    }

    void Scheduler::putTask(inner::Task *task)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if(unfinishedTasks.find(task) != std::end(unfinishedTasks))
        {
            if(task->canRun())
            {
                markReady(lock, *task);
                cond.notify_one();
            }
            else
            {
                waiting.insert(task);
            }
        }
    }

    void Scheduler::notifyPush(inner::Task &inst)
    {
        std::lock_guard<std::mutex> lock(mutex);
        updateChildrenReadiness(lock, inst);
    }

    void Scheduler::notifyPull(inner::Task &inst)
    {
        std::lock_guard<std::mutex> lock(mutex);
        updateReadiness(lock, inst);
    }

    void Scheduler::notifySelf(inner::Task &inst)
    {
        std::lock_guard<std::mutex> lock(mutex);
        updateReadiness(lock, inst);
    }

    void Scheduler::notifyFinished(inner::Task &inst)
    {
        std::lock_guard<std::mutex> lock(mutex);
        markFinished(lock, inst);
        if(unfinishedTasks.empty())
        {
            cond.notify_all();
        }
    }

    void Scheduler::updateReadiness(std::lock_guard<std::mutex> &lock,
        inner::Task &task)
    {
        auto iter = waiting.find(&task);
        if(iter != end(waiting))
        {
            if(unfinishedTasks.find(&task) != std::end(unfinishedTasks) &&
                task.canRun())
            {
                waiting.erase(iter);
                markReady(lock, task);
                cond.notify_one();
            }
        }
    }

    void Scheduler::updateChildrenReadiness(std::lock_guard<std::mutex> &lock,
        inner::Task &task)
    {
        auto depIters = childDeps.find(&task);
        if(depIters != std::end(childDeps))
        {
            for(auto *depTask : depIters->second)
            {
                assert(depTask);
                updateReadiness(lock, *depTask);
            }
        }
    }

    void Scheduler::updateParentsReadiness(std::lock_guard<std::mutex> &lock,
        inner::Task &task)
    {
        auto depIters = parentDeps.find(&task);
        if(depIters != std::end(parentDeps))
        {
            for(auto *depTask : depIters->second)
            {
                assert(depTask);
                updateReadiness(lock, *depTask);
            }
        }
    }

    void Scheduler::markReady(std::lock_guard<std::mutex>&, inner::Task &task)
    {
        auto priorityIter = priorities.find(&task);
        if(priorityIter == end(priorities))
            throw std::runtime_error("");
        ready.push(PrioritizedTask{&task, priorityIter->second});
    }

    void Scheduler::markFinished(std::lock_guard<std::mutex> &lock, inner::Task &task)
    {
        if(unfinishedTasks.erase(&task) > 0)
        {
            updateParentsReadiness(lock, task);
            updateChildrenReadiness(lock, task);
        }
    }

    template<typename Expand>
    Scheduler::TaskSet Scheduler::findTasks(inner::Node &node, Expand expand)
    {
        TaskSet result;
        NodeQueue front;
        front.push(&node);
        NodeSet seenNodes;
        while(!front.empty())
        {
            auto *const cur = front.front();
            front.pop();
            assert(cur);
            for(auto *p : expand(*cur))
            {
                if(seenNodes.insert(p).second)
                {
                    auto *const task = p->task();
                    if(task)
                        result.insert(task);
                    else
                        front.push(p);
                }
            }
        }
        return result;
    }

    Scheduler::TaskSet Scheduler::findParentTasks(inner::Node &node)
    {
        return findTasks(node, [](inner::Node &n){return n.parents();});
    }

    Scheduler::TaskSet Scheduler::findChildTasks(inner::Node &node)
    {
        return findTasks(node, [](inner::Node &n){return n.children();});
    }
}
