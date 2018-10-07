#ifndef XPIPE_INNER_NULLABLE_H
#define XPIPE_INNER_NULLABLE_H

#include <stdexcept>
#include <memory>

namespace xpipe
{
    namespace inner
    {
        template<typename T>
        class Nullable
        {
        public:
            Nullable();
            Nullable(const Nullable &that);
            Nullable(Nullable &&that);
            explicit Nullable(const T &value);
            explicit Nullable(T &&value);

            Nullable &operator=(const Nullable &that);
            Nullable &operator=(Nullable &&that);

            T &operator*();
            const T &operator*() const;

            T *operator->();
            const T *operator->() const;

            bool isNull() const
            {
                return !static_cast<bool>(value);
            }

        private:
            std::shared_ptr<T> value;
        };

        template<typename T>
        inline T extractNullable(const Nullable<T> &value)
        {
            return !value.isNull()?*value:T();
        }

        template<typename T>
        Nullable<T>::Nullable(const Nullable &that)
            :value(that.value
                ?std::make_shared<T>(*that.value):std::shared_ptr<T>())
        {}

        template<typename T>
        Nullable<T>::Nullable(Nullable &&that)
            :value(that.value
                ?std::make_shared<T>(std::move(*that.value)):std::shared_ptr<T>())
        {}

        template<typename T>
        Nullable<T>::Nullable()
            :value()
        {
        }

        template<typename T>
        Nullable<T>::Nullable(const T &value)
            :value(std::make_shared<T>(value))
        {
        }

        template<typename T>
        Nullable<T>::Nullable(T &&value)
            :value(std::make_shared<T>(std::move(value)))
        {
        }

        template<typename T>
        Nullable<T> &Nullable<T>::operator=(const Nullable &that)
        {
            if(that.value)
                this->value = std::make_shared<T>(*that.value);
            else
                this->value.reset();
            return *this;
        }

        template<typename T>
        Nullable<T> &Nullable<T>::operator=(Nullable &&that)
        {
            if(that.value)
                this->value = std::make_shared<T>(std::move(*that.value));
            else
                this->value.reset();
            return *this;
        }

        template<typename T>
        T &Nullable<T>::operator*()
        {
            if(value)
                return *value;
            throw std::invalid_argument("null object");
        }

        template<typename T>
        const T &Nullable<T>::operator*() const
        {
            return const_cast<Nullable<T>*>(this)->operator*();
        }

        template<typename T>
        T *Nullable<T>::operator->()
        {
            return &operator*();
        }

        template<typename T>
        const T *Nullable<T>::operator->() const
        {
            return const_cast<Nullable<T>*>(this)->operator->();
        }
    }
}

#endif
