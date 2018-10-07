#ifndef XPIPE_INNER_ASYNCQUEUE_H
#define XPIPE_INNER_ASYNCQUEUE_H

#include <cassert>
#include <cstddef>
#include <deque>
#include <functional>
#include <algorithm>
#include <mutex>
#include <condition_variable>

#include "xpipe/inner/Nullable.h"

namespace xpipe
{
    namespace inner
    {
        template<typename T>
        class AsyncQueue
        {
        public:
            AsyncQueue()
                :queue(), queueMutex()
            {
            }

            ~AsyncQueue();

            void push(const T &value);
            void push(T &&value);
            Nullable<T> tryPop();
            bool empty() const;
            std::size_t size() const;

            AsyncQueue(const AsyncQueue&) = delete;
            AsyncQueue &operator=(const AsyncQueue&) = delete;

        private:
            using Queue = std::deque<T>;

        private:
            Queue queue;
            mutable std::mutex queueMutex;
        };

        template<typename T>
        AsyncQueue<T>::~AsyncQueue()
        {}

        template<typename T>
        void AsyncQueue<T>::push(const T &value)
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            queue.push_back(value);
        }

        template<typename T>
        void AsyncQueue<T>::push(T &&value)
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            queue.push_back(std::move(value));
        }

        template<typename T>
        Nullable<T> AsyncQueue<T>::tryPop()
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if(!queue.empty())
            {
                Nullable<T> res(std::move(queue.front()));
                queue.pop_front();
                return res;
            }
            return Nullable<T>();
        }

        template<typename T>
        bool AsyncQueue<T>::empty() const
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            return queue.empty();
        }

        template<typename T>
        std::size_t AsyncQueue<T>::size() const
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            return queue.size();
        }
    }
}

#endif
