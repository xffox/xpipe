#ifndef XPIPE_PIPELINE_H
#define XPIPE_PIPELINE_H

#include <vector>
#include <thread>
#include <memory>
#include <cstddef>
#include <tuple>
#include <utility>
#include <tuple>

#include "xpipe/Runnable.h"
#include "xpipe/Stage.h"
#include "xpipe/Functional.h"
#include "xpipe/IndexSequence.h"
#include "xpipe/inner/StageTraits.h"
#include "xpipe/inner/MultiOutTypedNode.h"
#include "xpipe/inner/MultiProcTask.h"
#include "xpipe/inner/InTypedNode.h"
#include "xpipe/inner/Node.h"
#include "xpipe/inner/FromTaskNode.h"
#include "xpipe/inner/OrNode.h"
#include "xpipe/inner/AndNode.h"
#include "xpipe/inner/SeqNode.h"
#include "xpipe/inner/InterruptTaskNode.h"
#include "xpipe/inner/SinkTaskNode.h"
#include "xpipe/inner/graphptr.h"

namespace xpipe
{
    class Scheduler;

    class Pipeline
    {
    public:
        Pipeline(const BaseStage &stages);
        Pipeline(const BaseStage &stages, std::size_t threadCount);
        ~Pipeline();

        void run();
        void stop();

        Pipeline(const Pipeline&) = delete;
        Pipeline &operator=(const Pipeline&) = delete;

    private:
        class Routine
        {
        public:
            Routine(Scheduler &scheduler)
                :scheduler(&scheduler)
            {}

            void operator()();

        private:
            Scheduler *scheduler;
        };

        using ThreadCol = std::vector<std::shared_ptr<std::thread>>;

    private:
        std::size_t threadCount;
        inner::graphptr::NodePointer<inner::Node> node;
        std::unique_ptr<Scheduler> scheduler;
        ThreadCol threads;
    };
}

#endif
