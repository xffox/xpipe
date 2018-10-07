#ifndef XPIPE_INDEXSEQUENCE_H
#define XPIPE_INDEXSEQUENCE_H

#include <cstddef>

namespace xpipe
{
    template<std::size_t... I>
    struct IndexSequence
    {};

    namespace inner
    {
        template<std::size_t N, std::size_t... I>
        struct MakeIndexSequenceSz
        {
            using S = typename MakeIndexSequenceSz<N-1, N-1, I...>::S;
        };
        template<std::size_t... I>
        struct MakeIndexSequenceSz<0, I...>
        {
            using S = IndexSequence<I...>;
        };

        template<template<std::size_t...> class T,
            std::size_t N, std::size_t... I>
        struct MakeSequenceSz
        {
            using S = typename MakeSequenceSz<T, N-1, N-1, I...>::S;
        };
        template<template<std::size_t...> class T, std::size_t... I>
        struct MakeSequenceSz<T, 0, I...>
        {
            using S = T<I...>;
        };
    }

    template<std::size_t N>
    using MakeIndexSequence = typename inner::MakeIndexSequenceSz<N>::S;

    template<template<std::size_t...> class T, std::size_t N>
    using MakeSequence = typename inner::MakeSequenceSz<T, N>::S;
}

#endif
