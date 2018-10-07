#ifndef XPIPE_INNER_STAGETRAITS_H
#define XPIPE_INNER_STAGETRAITS_H

#include <tuple>
#include <type_traits>

#include "xpipe/Inlet.h"

namespace xpipe
{
    namespace inner
    {
        template<class F>
        struct StageTraits
        {
        private:
            using DF = typename std::decay<F>::type;

        public:
            using InType = typename StageTraits<decltype(&DF::operator())>::InType;
            using OutType = typename StageTraits<decltype(&DF::operator())>::OutType;

        private:
            StageTraits() = default;
        };

        template<class F, class IN, class OUT>
        struct StageTraits<bool(F::*)(IN, Inlet<OUT>&) const>
        {
            using InType = typename std::decay<IN>::type;
            using OutType = typename std::decay<OUT>::type;

        private:
            StageTraits() = default;
        };

        template<class F, class IN, class OUT>
        struct StageTraits<bool(F::*)(IN, Inlet<OUT>&)>
        {
            using InType = typename std::decay<IN>::type;
            using OutType = typename std::decay<OUT>::type;

        private:
            StageTraits() = default;
        };

        template<class IN, class OUT>
        struct StageTraits<bool(*)(IN, Inlet<OUT>&)>
        {
            using InType = typename std::decay<IN>::type;
            using OutType = typename std::decay<OUT>::type;

        private:
            StageTraits() = default;
        };

        template<class F>
        struct SourceStageTraits
        {
            using OutType = typename SourceStageTraits<decltype(&F::operator())>::OutType;

        private:
            SourceStageTraits() = default;
        };

        template<class F, class OUT>
        struct SourceStageTraits<bool(F::*)(Inlet<OUT>&) const>
        {
            using OutType = typename std::decay<OUT>::type;

        private:
            SourceStageTraits() = default;
        };

        template<class F, class OUT>
        struct SourceStageTraits<bool(F::*)(Inlet<OUT>&)>
        {
            using OutType = typename std::decay<OUT>::type;

        private:
            SourceStageTraits() = default;
        };

        template<class OUT>
        struct SourceStageTraits<bool(*)(Inlet<OUT>&)>
        {
            using OutType = typename std::decay<OUT>::type;

        private:
            SourceStageTraits() = default;
        };

        template<class F>
        struct SinkStageTraits
        {
            using InType = typename SinkStageTraits<decltype(&F::operator())>::InType;

        private:
            SinkStageTraits() = default;
        };

        template<class F, class IN>
        struct SinkStageTraits<bool(F::*)(IN) const>
        {
            using InType = typename std::decay<IN>::type;

        private:
            SinkStageTraits() = default;
        };

        template<class F, class IN>
        struct SinkStageTraits<bool(F::*)(IN)>
        {
            using InType = typename std::decay<IN>::type;

        private:
            SinkStageTraits() = default;
        };

        template<class IN>
        struct SinkStageTraits<bool(*)(IN)>
        {
            using InType = typename std::decay<IN>::type;

        private:
            SinkStageTraits() = default;
        };

        template<template<typename...> class T, class F>
        struct MultiOutStageTraits
        {
        private:
            using DF = typename std::decay<F>::type;

        public:
            using InType =
                typename MultiOutStageTraits<T, decltype(&DF::operator())>::InType;
            using TargetType =
                typename MultiOutStageTraits<T, decltype(&DF::operator())>::TargetType;
            using FuncType =
                typename MultiOutStageTraits<T, decltype(&DF::operator())>::FuncType;

        private:
            MultiOutStageTraits() = default;
        };

        template<template<typename...> class T, class F, class In, class... Outs>
        struct MultiOutStageTraits<T, bool(F::*)(In, Inlet<Outs>&...)>
        {
            using InType = typename std::decay<In>::type;
            using TargetType = T<typename std::decay<Outs>::type...>;
            using FuncType =
                T<typename std::decay<In>::type, typename std::decay<Outs>::type...>;

        private:
            MultiOutStageTraits() = default;
        };

        template<template<typename...> class T, class F, class In, class... Outs>
        struct MultiOutStageTraits<T, bool(F::*)(In, Inlet<Outs>&...) const>
        {
            using InType = typename std::decay<In>::type;
            using TargetType = T<typename std::decay<Outs>::type...>;
            using FuncType =
                T<typename std::decay<In>::type, typename std::decay<Outs>::type...>;

        private:
            MultiOutStageTraits() = default;
        };

        template<template<typename...> class T, class In, class... Outs>
        struct MultiOutStageTraits<T, bool(*)(In, Inlet<Outs>&...)>
        {
            using InType = typename std::decay<In>::type;
            using TargetType = T<typename std::decay<Outs>::type...>;
            using FuncType =
                T<typename std::decay<In>::type, typename std::decay<Outs>::type...>;

        private:
            MultiOutStageTraits() = default;
        };
    }
}

#endif
