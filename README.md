Framework for `C++` to express computations as graphs. Nodes represents simple
operations with state (called stages). Graph structure represents data dependencies
between stages. There is a scheduler which runs stages according to the readiness
of the input data. Multiple threads can execute the graph. Cycles are allowed in
the graph.

Graph is built using composition operators:
* combine `>>` - one stage after another;
* and `&&` - inputs from two stages;
* or `||` - input from one of two stages;
* `parmap` - any stage can handle input;
* other.

Example, Fibonacci numbers:

    const std::size_t sz = 42;
    std::vector<int> fib;
    auto f = map(
        [](const std::tuple<int, int> &val, Inlet<int> &inlet){
            inlet.push(std::get<0>(val) + std::get<1>(val));
            return true;
        });
    auto r = seq(source(
            stage::SequenceOf<int>{1, 1}), f)>>
        multimap(stage::CopyOf<int, 2>());
    r.get<0>()>>map(stage::Delay<int, 2>())>>f;
    r.get<1>()>>sink([&fib, sz](int v){
            fib.push_back(v);
            return fib.size() < sz;
        });
    Pipeline(f).run();
