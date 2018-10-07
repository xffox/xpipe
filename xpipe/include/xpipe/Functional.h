#ifndef XPIPE_FUNCTIONAL_H
#define XPIPE_FUNCTIONAL_H

namespace xpipe
{
    namespace inner
    {
        template<template<typename...> class T, typename Arg,
            std::size_t N, typename... Args>
        struct RepeatedArgs
        {
            using R = typename RepeatedArgs<T, Arg, N-1, Arg, Args...>::R;
        };
        template<template<typename...> class T, typename Arg,
            typename... Args>
        struct RepeatedArgs<T, Arg, 0, Args...>
        {
            using R = T<Args...>;
        };
    }

    template<typename F, typename R>
    R reduce(F, R r)
    {
        return r;
    }
    template<typename F, typename R, typename... Vals>
    R reduce(F func, R r, R v, Vals... args)
    {
        return reduce(func, func(r, v), args...);
    }

    struct Pass
    {
        template<typename... T>
        Pass(T...){}
    };

    template<template<typename...> class T, typename Arg, std::size_t C>
        using RepeatArgs = typename inner::RepeatedArgs<T, Arg, C>::R;
}

#endif
