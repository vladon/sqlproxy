#pragma once

#include <iostream>
#include <ostream>

namespace sql_proxy
{

class log
{
public:
    template<typename T>
    static void _(T t)
    {
        std::cout << t;
    }

    template<typename T, typename ... Args>
    static void _(T t, Args ... args)
    {
        std::cout << t;
        _(args...);
    }
};

}