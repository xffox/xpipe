#ifndef XPIPE_INNER_UTIL_H
#define XPIPE_INNER_UTIL_H

#include <memory>
#include <stdexcept>

namespace xpipe
{
    namespace inner
    {
        namespace util
        {
            template<typename T>
            inline std::unique_ptr<T> &notNull(std::unique_ptr<T> &value)
            {
                if(!value)
                    throw std::invalid_argument("null pointer");
                return value;
            }
        }
    }
}

#endif
