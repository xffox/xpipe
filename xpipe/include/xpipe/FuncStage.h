#ifndef XPIPE_FUNCSTAGE_H
#define XPIPE_FUNCSTAGE_H

#include <functional>

namespace xpipe
{
    template<typename Arg, typename R>
    class FuncStage
    {
    public:
        template<typename Func>
        FuncStage(Func func)
            :func(func)
        {}

        bool operator()(Arg value, Inlet<R> &inlet);

    private:
        std::function<R(Arg)> func;
    };

    template<typename Arg, typename R>
    bool FuncStage<Arg, R>::operator()(Arg value, Inlet<R> &inlet)
    {
        inlet.push(func(value));
        return true;
    }
}

#endif
