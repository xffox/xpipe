#include <iterator>
#include <algorithm>
#include <vector>
#include <tuple>
#include <cstddef>
#include <unordered_set>
#include <string>
#include <memory>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "xpipe/Pipeline.h"
#include "xpipe/Runnable.h"
#include "xpipe/stage/SequenceOf.h"
#include "xpipe/stage/CopyOf.h"
#include "xpipe/stage/Delay.h"

namespace xpipe
{
    namespace test
    {
        template<class Cont>
        class ContainerSource
        {
        public:
            ContainerSource(const Cont &cont)
                :iter(std::begin(cont)), end(std::end(cont))
            {}

            bool operator()(Inlet<typename Cont::value_type> &inlet)
            {
                if(iter != end)
                {
                    inlet.push(*iter);
                    ++iter;
                    return true;
                }
                return false;
            }

        private:
            typename Cont::const_iterator iter;
            typename Cont::const_iterator end;
        };

        template<class Cont>
        class ContainerSink
        {
        public:
            ContainerSink(Cont &cont)
                :iter(std::inserter(cont, std::end(cont)))
            {}

            bool operator()(typename Cont::value_type v)
            {
                *iter = v;
                ++iter;
                return true;
            }

        private:
            std::insert_iterator<Cont> iter;
        };

        class PipelineTest: public CppUnit::TestCase
        {
            CPPUNIT_TEST_SUITE(PipelineTest);
            CPPUNIT_TEST(testSourceToSink);
            CPPUNIT_TEST(testAllFunc);
            CPPUNIT_TEST(testAndOp);
            CPPUNIT_TEST(testAnyFunc);
            CPPUNIT_TEST(testOrOp);
            CPPUNIT_TEST(testSeqFunc);
            CPPUNIT_TEST(testMapFunc);
            CPPUNIT_TEST(testMultimapFunc);
            CPPUNIT_TEST(testParmapFunc);
            CPPUNIT_TEST(testSequenceOfStage);
            CPPUNIT_TEST(testCopyOfStage);
            CPPUNIT_TEST(testUseStage);
            CPPUNIT_TEST(testCycle);
            CPPUNIT_TEST_SUITE_END();

            using ValCol = std::vector<int>;
            using ValMultiset = std::unordered_multiset<int>;

        public:
            void testSourceToSink()
            {
                const ValCol values{1, 2, 42, 97, 113};
                ValCol act;
                auto f =
                    source(ContainerSource<ValCol>(values))
                    >>sink(ContainerSink<ValCol>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == values);
            }

            void testAllFunc()
            {
                using ResCol = std::vector<std::tuple<int, int, int>>;
                const ValCol values1{1, 2, 42, 97, 113};
                const ValCol values2{2, 3, 43, 98, 114, 256};
                const ValCol values3{3, 4, 5, 6, 7, 8, 9, 10};
                ResCol exp;
                for(std::size_t i = 0;
                    i < std::min(std::min(values1.size(), values2.size()),
                        values3.size());
                    ++i)
                {
                    exp.emplace_back(values1[i], values2[i], values3[i]);
                }
                ResCol act;
                auto f =
                    all(
                        source(ContainerSource<ValCol>(values1)),
                        source(ContainerSource<ValCol>(values2)),
                        source(ContainerSource<ValCol>(values3)))
                    >>sink(ContainerSink<ResCol>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testAndOp()
            {
                using ResCol = std::vector<std::tuple<int, int>>;
                const ValCol values1{1, 2, 42, 97, 113};
                const ValCol values2{2, 3, 43, 98, 114, 256};
                ResCol exp;
                for(std::size_t i = 0;
                    i < std::min(values1.size(), values2.size());
                    ++i)
                {
                    exp.emplace_back(values1[i], values2[i]);
                }
                ResCol act;
                auto f =
                    (source(ContainerSource<ValCol>(values1)) &&
                        source(ContainerSource<ValCol>(values2)))
                    >>sink(ContainerSink<ResCol>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testAnyFunc()
            {
                const ValCol values1{1, 2, 42, 97, 113};
                const ValCol values2{2, 3, 43, 98, 114, 256};
                const ValCol values3{5, 6, 7, 8, 9, 10, 11, 12};
                ValMultiset exp;
                exp.insert(std::begin(values1), std::end(values1));
                exp.insert(std::begin(values2), std::end(values2));
                exp.insert(std::begin(values3), std::end(values3));
                ValMultiset act;
                auto f =
                    any(
                        source(ContainerSource<ValCol>(values1)),
                        source(ContainerSource<ValCol>(values2)),
                        source(ContainerSource<ValCol>(values3)))
                    >>sink(ContainerSink<ValMultiset>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testOrOp()
            {
                const ValCol values1{1, 2, 42, 97, 113};
                const ValCol values2{2, 3, 43, 98, 114, 256};
                ValMultiset exp;
                exp.insert(std::begin(values1), std::end(values1));
                exp.insert(std::begin(values2), std::end(values2));
                ValMultiset act;
                auto f =
                    (source(ContainerSource<ValCol>(values1)) ||
                        source(ContainerSource<ValCol>(values2)))
                    >>sink(ContainerSink<ValMultiset>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testSeqFunc()
            {
                const ValCol values1{1, 2, 42, 97, 113};
                const ValCol values2{2, 3, 43, 98, 114, 256};
                const ValCol values3{5, 6, 7, 8, 9, 10, 11, 12};
                ValCol exp;
                exp.insert(std::end(exp), std::begin(values1), std::end(values1));
                exp.insert(std::end(exp), std::begin(values2), std::end(values2));
                exp.insert(std::end(exp), std::begin(values3), std::end(values3));
                ValCol act;
                auto f =
                    seq(
                        source(ContainerSource<ValCol>(values1)),
                        source(ContainerSource<ValCol>(values2)),
                        source(ContainerSource<ValCol>(values3)))
                    >>sink(ContainerSink<ValCol>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testMapFunc()
            {
                using ResCol = std::vector<std::string>;
                const ValCol values{1, 2, 42, 97, 113};
                auto m = [](int v){
                    return std::to_string(v+1);
                };
                ResCol exp;
                for(auto v : values)
                {
                    exp.push_back(m(v));
                }
                ResCol act;
                auto f =
                    source(ContainerSource<ValCol>(values))
                    >>map([m](int v, Inlet<std::string> &inlet){
                            inlet.push(m(v));
                            return true;
                        })
                    >>sink(ContainerSink<ResCol>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testMultimapFunc()
            {
                const ValCol values{1, 2, 42, 97, 113};
                auto m = [](int v){
                    return std::make_tuple(v+1, std::to_string(v+2), v>2);
                };
                ValCol exp1;
                std::vector<std::string> exp2;
                std::vector<bool> exp3;
                for(auto v : values)
                {
                    const auto res = m(v);
                    exp1.push_back(std::get<0>(res));
                    exp2.push_back(std::get<1>(res));
                    exp3.push_back(std::get<2>(res));
                }
                ValCol act1;
                std::vector<std::string> act2;
                std::vector<bool> act3;
                auto f =
                    source(ContainerSource<ValCol>(values))
                    >>multimap([m](int v,
                                Inlet<int> &inlet1,
                                Inlet<std::string> &inlet2,
                                Inlet<bool> &inlet3){
                            const auto res = m(v);
                            inlet1.push(std::get<0>(res));
                            inlet2.push(std::get<1>(res));
                            inlet3.push(std::get<2>(res));
                            return true;
                        });
                f.get<0>()>>sink(ContainerSink<decltype(act1)>(act1));
                f.get<1>()>>sink(ContainerSink<decltype(act2)>(act2));
                f.get<2>()>>sink(ContainerSink<decltype(act3)>(act3));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act1 == exp1);
                CPPUNIT_ASSERT(act2 == exp2);
                CPPUNIT_ASSERT(act3 == exp3);
            }

            void testParmapFunc()
            {
                using StringMultiset = std::unordered_multiset<std::string>;
                using StringCol = std::vector<std::string>;
                const ValCol values{1, 2, 42, 97, 113};
                auto m = [](int v){
                    return std::to_string(v+1);
                };
                StringMultiset exp;
                for(auto v : values)
                {
                    exp.insert(m(v));
                }
                StringCol act1;
                StringCol act2;
                StringCol act3;
                StringMultiset act;
                auto sm = [m](int v, Inlet<std::string> &inlet){
                    inlet.push(m(v));
                    return true;
                };
                auto f =
                    source(ContainerSource<ValCol>(values))
                    >>parmap(
                        map(sm),
                        map(sm),
                        map(sm));
                f.get<0>()>>sink(ContainerSink<StringCol>(act1));
                f.get<1>()>>sink(ContainerSink<StringCol>(act2));
                f.get<2>()>>sink(ContainerSink<StringCol>(act3));
                Pipeline(f).run();
                act.insert(std::begin(act1), std::end(act1));
                act.insert(std::begin(act2), std::end(act2));
                act.insert(std::begin(act3), std::end(act3));
                CPPUNIT_ASSERT(act == exp);
            }

            void testSequenceOfStage()
            {
                const ValCol exp{1, 2, 3, 42, 97, 113};
                ValCol act;
                auto f =
                    source(stage::SequenceOf<int>{1, 2, 3, 42, 97, 113})
                    >>sink(ContainerSink<ValCol>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testCopyOfStage()
            {
                const ValCol values{1, 2, 3, 42, 97, 113};
                auto f =
                    source(ContainerSource<ValCol>(values))
                    >>multimap(stage::CopyOf<int, 3>());
                ValCol act1;
                ValCol act2;
                ValCol act3;
                f.get<0>()>>sink(ContainerSink<ValCol>(act1));
                f.get<1>()>>sink(ContainerSink<ValCol>(act2));
                f.get<2>()>>sink(ContainerSink<ValCol>(act3));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act1 == values);
                CPPUNIT_ASSERT(act2 == values);
                CPPUNIT_ASSERT(act3 == values);
            }

            void testUseStage()
            {
                class ValRunnable: public Runnable<int>
                {
                public:
                    ValRunnable(const ValCol &values)
                        :values(values), iter(std::begin(this->values))
                    {}
                    void init(RunnableInlet<int> &inlet) override
                    {
                        this->inlet = &inlet;
                    }
                    void destroy() override
                    {
                        inlet = nullptr;
                    }
                    bool canRun() override
                    {
                        return true;
                    }
                    bool run() override
                    {
                        if(iter != std::end(values))
                        {
                            assert(inlet);
                            inlet->push(*iter);
                            ++iter;
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                private:
                    RunnableInlet<int> *inlet = nullptr;
                    ValCol values;
                    ValCol::const_iterator iter;
                };
                const ValCol exp{1, 2, 3, 42, 97, 113};
                ValCol act;
                auto f = use(
                    std::unique_ptr<Runnable<int>>(new ValRunnable(exp)))
                    >>sink(ContainerSink<ValCol>(act));
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }

            void testCycle()
            {
                const ValCol exp{1, 1, 2, 3, 5, 8, 13, 21, 34};
                const auto sz = exp.size();
                ValCol act;
                auto f = map(
                    [](const std::tuple<int, int> &val, Inlet<int> &inlet){
                        inlet.push(std::get<0>(val) + std::get<1>(val));
                        return true;
                    });
                auto r = seq(source(
                        stage::SequenceOf<int>{1, 1}), f)>>
                    multimap(stage::CopyOf<int, 2>());
                r.get<0>()>>map(stage::Delay<int, 2>())>>f;
                r.get<1>()>>sink([&act, sz](int v){
                        act.push_back(v);
                        return act.size() < sz;
                    });
                Pipeline(f).run();
                CPPUNIT_ASSERT(act == exp);
            }
        };
        CPPUNIT_TEST_SUITE_REGISTRATION(PipelineTest);
    }
}
