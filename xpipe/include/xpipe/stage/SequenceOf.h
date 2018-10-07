#ifndef XPIPE_STAGE_SEQUENCEOF_H
#define XPIPE_STAGE_SEQUENCEOF_H

#include <cstddef>
#include <vector>

#include "xpipe/Inlet.h"

namespace xpipe
{
    namespace stage
    {
        template<typename T>
        class SequenceOf
        {
        public:
            SequenceOf(std::initializer_list<T> values)
                :idx(0), values(values)
            {}

            SequenceOf(SequenceOf&&) = default;
            SequenceOf(const SequenceOf&) = delete;
            SequenceOf &operator=(SequenceOf&&) = default;
            SequenceOf &operator=(const SequenceOf&) = delete;

            bool operator()(xpipe::Inlet<T> &inlet)
            {
                if(idx < values.size())
                {
                    inlet.push(std::move(values[idx++]));
                    return true;
                }
                return false;
            }

        private:
            std::size_t idx;
            std::vector<T> values;
        };
    }
}

#endif
