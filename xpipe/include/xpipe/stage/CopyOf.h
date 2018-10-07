#ifndef XPIPE_STAGE_COPYOF_H
#define XPIPE_STAGE_COPYOF_H

#include "xpipe/Inlet.h"
#include "xpipe/Functional.h"

namespace xpipe
{
    namespace inner
    {
        template<typename T, typename... Ts>
        struct CopyOfFunc
        {
            bool operator()(T val,
                xpipe::Inlet<T> &inlet, xpipe::Inlet<Ts>&... inlets) const
            {
                Pass{(inlet.push(val),nullptr), (inlets.push(val),nullptr)...};
                return true;
            }
        };
    }
    namespace stage
    {
        template<typename Arg, std::size_t C>
        using CopyOf = RepeatArgs<inner::CopyOfFunc, Arg, C>;
    }
}

#endif
