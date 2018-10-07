#include <cstddef>
#include <string>
#include <tuple>
#include <cctype>
#include <iostream>

#include "xpipe/Pipeline.h"
#include "xpipe/Stage.h"
#include "xpipe/stage/SequenceOf.h"
#include "xpipe/stage/IterateOver.h"

namespace
{
    std::string abbreviate(const std::string &value)
    {
        if(value.empty())
            return value;
        std::string r;
        xpipe::Pipeline(
            (xpipe::source(xpipe::stage::iterateOver(std::begin(value), std::end(value))) &&
             xpipe::seq(
                 xpipe::source(xpipe::stage::SequenceOf<char>{'\0'}),
                 xpipe::source(xpipe::stage::iterateOver(std::begin(value), std::end(value))))
            )
            >>xpipe::map([](std::tuple<char, char> c, xpipe::Inlet<char> &inlet) {
                if(std::isalpha(std::get<0>(c)) && !std::isalpha(std::get<1>(c)))
                {
                    inlet.push(std::toupper(std::get<0>(c)));
                }
                return true;
            })
            >>xpipe::sink([&r](char c) {
                r.push_back(c);
                return true;
            })).run();

        return r;
    }
}

int main(int argc, const char *argv[])
{
    if(argc != 2)
    {
        std::cerr<<"usage: "<<argv[0]<<" <phrase>"<<std::endl;
        return 1;
    }
    std::cout<<argv[1]<<std::endl;
    std::cout<<abbreviate(argv[1])<<std::endl;
    return 0;
}
