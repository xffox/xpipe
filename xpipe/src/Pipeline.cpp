#include "xpipe/Pipeline.h"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <queue>
#include <unordered_set>

#include "xpipe/inner/Task.h"
#include "xpipe/inner/Node.h"
#include "Scheduler.h"

namespace xpipe
{
    namespace
    {
        template<class F>
        void traverseTasks(inner::Node &root, F func)
        {
            using NodeSet = std::unordered_set<inner::Node*>;
            using NodeQueue = std::queue<inner::Node*>;
            NodeQueue front;
            front.push(&root);
            NodeSet seen{&root};
            while(!front.empty())
            {
                auto *node = front.front();
                front.pop();
                assert(node);
                auto *task = node->task();
                if(task != nullptr)
                {
                    func(*task);
                }
                for(auto *parent : node->parents())
                {
                    assert(parent);
                    if(seen.insert(parent).second)
                        front.push(parent);
                }
                for(auto *child : node->children())
                {
                    assert(child);
                    if(seen.insert(child).second)
                        front.push(child);
                }
            }
        }
    }

    Pipeline::Pipeline(const BaseStage &stages)
        :Pipeline(stages, std::thread::hardware_concurrency())
    {}

    Pipeline::Pipeline(const BaseStage &stages, std::size_t threadCount)
        :threadCount(threadCount), node(stages.getNode()),
        scheduler(new Scheduler(stages.getNode()))
    {
        if(threadCount == 0)
            throw std::invalid_argument("thread count is 0");
    }

    Pipeline::~Pipeline()
    {}

    void Pipeline::run()
    {
        auto task = node;
        assert(task);
        traverseTasks(*task, [](inner::Task &task) {
                task.init();
            });
        scheduler->start();
        Routine routine(*scheduler);
        for(std::size_t i = 0; i < threadCount; ++i)
        {
            threads.push_back(std::make_shared<std::thread>(routine));
        }
        routine();
        for(auto &t : threads)
        {
            assert(t);
            t->join();
        }
        threads.clear();
        traverseTasks(*node, [](inner::Task &task) {
                task.destroy();
            });
    }

    void Pipeline::stop()
    {
        assert(scheduler);
        scheduler->stop();
    }

    void Pipeline::Routine::operator()()
    {
        assert(scheduler);
        while(true)
        {
            auto *task = scheduler->takeTask();
            if(task)
            {
                while(task->run())
                    ;
                scheduler->putTask(task);
            }
            else
            {
                break;
            }
        }
    }
}
