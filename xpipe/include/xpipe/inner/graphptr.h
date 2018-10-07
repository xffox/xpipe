#ifndef XPIPE_INNER_GRAPHPTR_H
#define XPIPE_INNER_GRAPHPTR_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <iterator>

namespace xpipe
{
    namespace inner
    {
        // pointer instances themselves are not multithreaded
        namespace graphptr
        {
            class Storage;

            template<typename T>
            class LinkPointer;

            template<typename T>
            class NodePointer
            {
                template<typename P>
                friend class LinkPointer;
                template<typename P>
                friend class NodePointer;
            public:
                NodePointer();
                NodePointer(T *data);
                NodePointer(std::unique_ptr<T> data);
                ~NodePointer();
                NodePointer(const NodePointer &that);
                NodePointer(NodePointer &&that);
                NodePointer &operator=(const NodePointer &that);
                NodePointer &operator=(NodePointer &&that);
                template<typename P>
                NodePointer(const NodePointer<P> &that);
                template<typename P>
                NodePointer(NodePointer<P> &&that);

                T &operator*();
                const T &operator*() const
                {
                    return *const_cast<NodePointer&>(*this);
                }
                T *operator->()
                {
                    return data;
                }
                const T *operator->() const
                {
                    return const_cast<NodePointer&>(*this).operator->();
                }

                operator bool() const;

                template<typename P>
                LinkPointer<P> link(const NodePointer<P> &dst);

            private:
                Storage *storage;
                T *data;
            };

            class BaseLinkPointer
            {
                friend class Storage;
            public:
                virtual ~BaseLinkPointer() = default;

            private:
                virtual void forget() = 0;
            };

            template<typename T>
            class LinkPointer: public BaseLinkPointer
            {
                template<typename P>
                friend class NodePointer;
                template<typename P>
                friend class LinkPointer;
            public:
                LinkPointer();
                LinkPointer(const LinkPointer &that);
                LinkPointer(LinkPointer &&that);
                LinkPointer &operator=(const LinkPointer &that);
                LinkPointer &operator=(LinkPointer &&that);
                template<typename P>
                LinkPointer(const LinkPointer<P> &that);
                template<typename P>
                LinkPointer(LinkPointer<P> &&that);
                ~LinkPointer();

                T &operator*();
                const T &operator*() const
                {
                    return *const_cast<LinkPointer&>(*this);
                }
                T *operator->()
                {
                    return dst;
                }
                const T *operator->() const
                {
                    return const_cast<LinkPointer&>(*this).operator->();
                }

                T *get() const;

                operator bool() const;

            private:
                template<typename P>
                LinkPointer(const NodePointer<P> &src, const NodePointer<T> &dst);

                void increment();
                void decrement();

                void forget() override;

            private:
                T *dst;
                Storage *dstStorage;
                Storage *srcStorage;
            };

            template<typename T, typename... Args>
            NodePointer<T> make_node(Args... args)
            {
                return NodePointer<T>(new T(std::forward<Args>(args)...));
            }

            class Storage
            {
                template<typename P>
                friend class NodePointer;
                template<typename P>
                friend class LinkPointer;
                template<typename P>
                friend class DataStorage;
            public:
                virtual ~Storage() = default;

            private:
                Storage()
                    :counter(1), traversers(0), neighbours{}, mutex()
                {}

                void increment()
                {
                    counter.fetch_add(1);
                }

                bool decrement()
                {
                    std::vector<Storage*> deferred;
                    bool dropped = false;
                    std::unordered_set<Storage*> seen;
                    traversers.fetch_add(1);
                    const auto prevCount = counter.fetch_sub(1);
                    if(prevCount == 1)
                    {
                        std::queue<Storage*> front;
                        front.push(this);
                        seen.insert(this);
                        while(!front.empty())
                        {
                            auto *cur = front.front();
                            front.pop();
                            assert(cur);
                            // no locking since no one owns the pointer
                            for(auto p : cur->neighbours)
                            {
                                auto *n = p.first;
                                assert(n);
                                if(seen.insert(n).second)
                                {
                                    const auto count = n->counter.load();
                                    if(count == 0)
                                    {
                                        if(n->traversers.load() > 0)
                                        {
                                            if(n < this)
                                            {
                                                deferred.push_back(n);
                                            }
                                            else
                                            {
                                                dropped = true;
                                                break;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        dropped = true;
                                        break;
                                    }
                                    front.push(n);
                                }
                            }
                        }
                    }
                    else
                    {
                        dropped = true;
                    }
                    traversers.fetch_sub(1);
                    if(!dropped)
                    {
                        for(auto *d : deferred)
                        {
                            assert(d);
                            while(d->traversers.load() > 0)
                                ;
                        }
                        for(auto *p : seen)
                        {
                            assert(p);
                            p->forgetLinks();
                        }
                        for(auto *p : seen)
                        {
                            delete p;
                        }
                        return true;
                    }
                    return false;
                }

                void forgetLinks()
                {
                    for(const auto &n : neighbours)
                    {
                        for(auto *p : n.second)
                        {
                            p->forget();
                        }
                    }
                    neighbours.clear();
                }

                std::atomic<std::size_t> counter;
                std::atomic<std::size_t> traversers;
                std::unordered_map<Storage*, std::unordered_set<BaseLinkPointer*>> neighbours;
                std::mutex mutex;
            };

            template<typename T>
            class DataStorage: public Storage
            {
                template<typename P>
                friend class NodePointer;

            public:
                ~DataStorage()
                {
                    delete data;
                }

            private:
                DataStorage(T *data)
                    :data(data)
                {}

            private:
                T *data;
            };

            template<typename T>
            NodePointer<T>::NodePointer()
                :storage()
            {}

            template<typename T>
            NodePointer<T>::NodePointer(T *data)
                :storage(new DataStorage<T>(data)), data(data)
            {}

            template<typename T>
            NodePointer<T>::NodePointer(std::unique_ptr<T> data)
                :NodePointer(data.get())
            {
                data.release();
            }

            template<typename T>
            NodePointer<T>::~NodePointer()
            {
                if(storage)
                    storage->decrement();
            }

            template<typename T>
            NodePointer<T>::NodePointer(const NodePointer &that)
                :storage(that.storage), data(that.data)
            {
                if(storage)
                    storage->increment();
            }

            template<typename T>
            NodePointer<T>::NodePointer(NodePointer &&that)
                :storage(that.storage), data(that.data)
            {
                that.storage = nullptr;
                that.data = nullptr;
            }

            template<typename T>
            template<typename P>
            NodePointer<T>::NodePointer(const NodePointer<P> &that)
                :storage(that.storage), data(that.data)
            {
                if(storage)
                    storage->increment();
            }

            template<typename T>
            template<typename P>
            NodePointer<T>::NodePointer(NodePointer<P> &&that)
                :storage(that.storage), data(that.data)
            {
                that.storage = nullptr;
                that.data = nullptr;
            }

            template<typename T>
            NodePointer<T> &NodePointer<T>::operator=(const NodePointer &that)
            {
                if(that.storage)
                    that.storage->increment();
                if(storage)
                    storage->decrement();
                this->storage = that.storage;
                this->data = that.data;
            }

            template<typename T>
            NodePointer<T> &NodePointer<T>::operator=(NodePointer &&that)
            {
                if(storage)
                    storage->decrement();
                this->storage = that.storage;
                this->data = that.data;
                that.storage = nullptr;
                that.data = nullptr;
                return *this;
            }

            template<typename T>
            template<typename P>
            LinkPointer<P> NodePointer<T>::link(const NodePointer<P> &that)
            {
                return LinkPointer<P>(*this, that);
            }

            template<typename T>
            T &NodePointer<T>::operator*()
            {
                return *data;
            }

            template<typename T>
            NodePointer<T>::operator bool() const
            {
                return data;
            }

            template<typename T>
            LinkPointer<T>::LinkPointer()
                :dst(nullptr), dstStorage(nullptr), srcStorage(nullptr)
            {}

            template<typename T>
            template<typename P>
            LinkPointer<T>::LinkPointer(
                const NodePointer<P> &src, const NodePointer<T> &dst)
                :dst(dst.data), dstStorage(dst.storage), srcStorage(src.storage)
            {
                if(!dstStorage || !srcStorage)
                {
                    this->dst = nullptr;
                    dstStorage = nullptr;
                    srcStorage = nullptr;
                }
                increment();
            }

            template<typename T>
            LinkPointer<T>::LinkPointer(const LinkPointer &that)
                :dst(that.dst),
                dstStorage(that.dstStorage), srcStorage(that.srcStorage)
            {
                increment();
            }

            template<typename T>
            LinkPointer<T>::LinkPointer(LinkPointer &&that)
                :dst(that.dst),
                dstStorage(that.dstStorage), srcStorage(that.srcStorage)
            {
                increment();
                that.decrement();
            }

            template<typename T>
            LinkPointer<T> &LinkPointer<T>::operator=(const LinkPointer &that)
            {
                // TODO: what if it's self reference
                decrement();
                dst = that.dst;
                dstStorage = that.dstStorage;
                srcStorage = that.srcStorage;
                increment();
                return *this;
            }

            template<typename T>
            LinkPointer<T> &LinkPointer<T>::operator=(LinkPointer &&that)
            {
                decrement();
                dst = that.dst;
                dstStorage = that.dstStorage;
                srcStorage = that.srcStorage;
                increment();
                that.decrement();
                return *this;
            }

            template<typename T>
            template<typename P>
            LinkPointer<T>::LinkPointer(const LinkPointer<P> &that)
                :dst(that.dst),
                dstStorage(that.dstStorage), srcStorage(that.srcStorage)
            {
                increment();
            }

            template<typename T>
            template<typename P>
            LinkPointer<T>::LinkPointer(LinkPointer<P> &&that)
                :dst(that.dst),
                dstStorage(that.dstStorage), srcStorage(that.srcStorage)
            {
                increment();
                that.decrement();
            }

            template<typename T>
            LinkPointer<T>::~LinkPointer()
            {
                decrement();
            }

            template<typename T>
            T &LinkPointer<T>::operator*()
            {
                return *dst;
            }

            template<typename T>
            LinkPointer<T>::operator bool() const
            {
                return dst;
            }

            template<typename T>
            T *LinkPointer<T>::get() const
            {
                return dst;
            }

            template<typename T>
            void LinkPointer<T>::increment()
            {
                if(dst)
                {
                    assert(srcStorage);
                    assert(dstStorage);
                    std::lock_guard<std::mutex> lock(dstStorage->mutex);
                    dstStorage->neighbours[srcStorage].insert(this);
                }
            }

            template<typename T>
            void LinkPointer<T>::decrement()
            {
                if(dst)
                {
                    assert(srcStorage);
                    assert(dstStorage);
                    // TODO: optimize
                    dstStorage->increment();
                    {
                        std::lock_guard<std::mutex> lock(dstStorage->mutex);
                        auto iter = dstStorage->neighbours.find(srcStorage);
                        assert(iter != std::end(dstStorage->neighbours));
                        const auto r = iter->second.erase(this);
                        (void)r;
                        assert(r == 1);
                        if(iter->second.empty())
                            dstStorage->neighbours.erase(iter);
                    }
                    // seek and destroy
                    dstStorage->decrement();
                    dst = nullptr;
                    dstStorage = nullptr;
                    srcStorage = nullptr;
                }
            }

            template<typename T>
            void LinkPointer<T>::forget()
            {
                dst = nullptr;
                srcStorage = nullptr;
                dstStorage = nullptr;
            }
        }
    }
}

#endif
