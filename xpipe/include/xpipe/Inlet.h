#ifndef XPIPE_INLET_H
#define XPIPE_INLET_H

namespace xpipe
{
    template<typename T>
    class Inlet
    {
    public:
        virtual ~Inlet() = default;
        virtual void push(T value) = 0;
    };
}

#endif
