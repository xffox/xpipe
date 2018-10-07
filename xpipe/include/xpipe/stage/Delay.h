#ifndef XPIPE_STAGE_DELAY_H
#define XPIPE_STAGE_DELAY_H

#include <cstddef>
#include <vector>
#include <tuple>

#include "xpipe/Inlet.h"
#include "xpipe/IndexSequence.h"
#include "xpipe/Functional.h"

namespace xpipe
{
    namespace stage
    {
        template<typename T, std::size_t C>
        class Delay
        {
            using Tuple = xpipe::RepeatArgs<std::tuple, T, C>;
        public:
            bool operator()(T val, xpipe::Inlet<Tuple> &inlet)
            {
                if(values.size() < C)
                {
                    values.push_back(std::move(val));
                }
                else
                {
                    values[(offset+C)%C] = std::move(val);
                }
                if(values.size() == C)
                {
                    push(inlet, xpipe::MakeIndexSequence<C>());
                    offset = (offset+1)%C;
                }
                return true;
            }

        private:
            using ValCol = std::vector<T>;

        private:
            template<std::size_t... Is>
            void push(xpipe::Inlet<Tuple> &inlet, xpipe::IndexSequence<Is...>)
            {
                inlet.push(std::make_tuple(values[(offset+Is)%C]...));
            }

        private:
            ValCol values;
            std::size_t offset = 0;
        };
    }
}

#endif
