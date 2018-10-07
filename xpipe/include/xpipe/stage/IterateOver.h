#ifndef XPIPE_STAGE_ITERATEOVER_H
#define XPIPE_STAGE_ITERATEOVER_H

#include <type_traits>

#include "xpipe/Inlet.h"

namespace xpipe
{
    namespace stage
    {
        template<typename Iter>
        class IterateOver
        {
            template<typename I>
            friend IterateOver<I> iterateOver(I begin, I end);
        public:
            bool operator()(
                xpipe::Inlet<typename std::decay<decltype(*Iter())>::type> &inlet)
            {
                if(begin != end)
                {
                    inlet.push(*begin);
                    ++begin;
                    return true;
                }
                return false;
            }

        private:
            IterateOver(Iter begin, Iter end)
                :begin(begin), end(end)
            {}

        private:
            Iter begin;
            Iter end;
        };

        template<typename Iter>
        IterateOver<Iter> iterateOver(Iter begin, Iter end)
        {
            return IterateOver<Iter>(begin, end);
        }
    }
}

#endif
