#pragma once

#include <iostream>
#include <tuple>
#include <variant>
#include <type_traits>

#ifndef NDEBUG

    #define DBG_TO(...) ::dbg::print(__VA_ARGS__)
    #define DBG(...) DBG_TO(std::cout, __VA_ARGS__)

#else

    #define DBG_TO(os, ...) os
    #define DBG(...) 

#endif
namespace dbg {


template<typename...Args>
inline std::ostream& print(std::ostream& os, Args&&...args);

template<typename Arg>
inline std::ostream& print_it(std::ostream& os, Arg&& arg);



template<typename C, std::void_t<
    decltype(std::begin(std::declval<C>())), 
    decltype(std::end(std::declval<C>()))
>* = nullptr>
inline std::ostream& operator<<(std::ostream& os, C const& c) {
    os << "{";
    bool first = true;
    for(auto const& e : c) {
        if (!first) {
            os << ", ";
        }
        first = false;
        print_it(os, e);
    }
    return os << "}";
}



template<typename...Ts>
inline std::ostream& operator<<(std::ostream& os, std::variant<Ts...> const& c) {
    os << "{#" << c.index() << ": ";
    return std::visit([&os] (auto&& v) -> std::ostream& { return print_it(os, std::forward<decltype(v)>(v)); }, c) << '}';
}



template<typename...Ts>
inline std::ostream& operator<<(std::ostream& os, std::tuple<Ts...> const& c) {
    auto f = [&os] (auto&& arg, auto&&...args) -> std::ostream& {
        print_it(os, std::forward<decltype(arg)>(arg));
        (print_it(os << ", ", std::forward<decltype(args)>(args)), ...);
        return os;
    };
    os << '(';
    return std::apply(f, c) << ')';
}

template<typename...Ts>
inline std::ostream& operator<<(std::ostream& os, std::pair<Ts...> const& c) {
    auto f = [&os] (auto&& arg, auto&&...args) -> std::ostream& {
        print_it(os, std::forward<decltype(arg)>(arg));
        (print_it(os << ", ", std::forward<decltype(args)>(args)), ...);
        return os;
    };
    os << '(';
    return std::apply(f, c) << ')';
}



template<typename...Args>
inline std::ostream& print(std::ostream& os, Args&&...args) {
    using ::dbg::operator<<;
    return (os << ... << std::forward<Args>(args)) << '\n';
}

template<typename Arg>
inline std::ostream& print_it(std::ostream& os, Arg&& arg) {
    using ::dbg::operator<<;
    return os << std::forward<Arg>(arg);
}

}