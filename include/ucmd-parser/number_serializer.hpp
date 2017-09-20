// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <cerrno>    
#include <cstdlib>

#include <memory>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <string_view> 

#include <boost/config.hpp>

#include "serializer.hpp"

namespace ucmdp::detail
{
    

template< typename T, T(*rdrFunc)(const char *, char **, int) >
struct integer_serializer
{
    static_assert(std::numeric_limits<T>::is_integer,
        "integer_serializer is only meaningful for integer types");

    static constexpr std::size_t max_stack_alloc = 72;

    // no inline because of alloca usage
    BOOST_NOINLINE void deserialize(std::string_view str, T &dest) const
    {
        if (str.empty())
        {
            throw "expected integer, but got an empty string";
        }
        std::unique_ptr<char[]> mem_holder;
        if (str.back() != '\0')
        {
            char *mem;
            if (str.size() < max_stack_alloc)
            {
                mem = reinterpret_cast<char *>(alloca(str.size() + 1));
            }
            else
            {
                mem_holder = std::make_unique<char[]>(str.size() + 1);
                mem = mem_holder.get();
            }
            memcpy(mem, str.data(), str.size());
            mem[str.size()] = '\0';
            str = std::string_view{ mem, str.size() };
        }

        char *end;
        auto tmp = rdrFunc(str.data(), &end, base);
        if (errno == ERANGE)
        {
            if (tmp == std::numeric_limits<T>::max())
            {
                throw "integer overflow";
            }
            if (tmp == std::numeric_limits<T>::min())
            {
                throw "integer underflow";
            }
            throw "undefined erange failure";
        }
        if (str.data() + str.size() != end)
        {
            throw "couldn't convert a string to an integer type";
        }
        dest = tmp;
    }

protected:
    constexpr explicit integer_serializer(int base)
        : base(base)
    {
        if (base != 0 && (base < 2 || base > 36))
        {
            throw std::invalid_argument("base is neither 0 nor in the interval [2, 36]");
        }
    }
    ~integer_serializer() = default;

private:
    const int base;
};

using longlong_serializer = integer_serializer<long long, strtoll>;
using long_serializer = integer_serializer<long, strtol>;
using ulonglong_serializer = integer_serializer<unsigned long long, strtoull>;
using ulong_serializer = integer_serializer<unsigned long, strtoul>;

template< typename T >
struct small_integer_serializer
    : private std::conditional_t< std::is_signed_v<T>
    , long_serializer
    , ulong_serializer
    >
{
private:
    using base_t = std::conditional_t< std::is_signed_v<T>,
        long_serializer,
        ulong_serializer
    >;

public:
    using argument_type = T;

    constexpr explicit small_integer_serializer(int base)
        : base_t(base)
    {
    }

    void deserialize(std::string_view str, T &dest) const
    {
        typename base_t::argument_type tmp;
        base_t::deserialize(str, tmp);
        if (tmp > std::numeric_limits<T>::max())
        {
            //TODO: throw appropriate exception
            throw "small int overflow";
        }
        if (std::is_signed<T>::value
            && tmp < std::numeric_limits<T>::min())
        {
            throw "small int underflow";
        }
        dest = static_cast<T>(tmp);
    }
};


template< typename T, T(*rdrFunc)(const char *, char**) >
struct floating_point_serializer
{
    static_assert(std::numeric_limits<T>::is_iec559,
        "floating_point_serializer is only meaningful for floating point types");

    static constexpr std::size_t max_stack_alloc = 72;

    BOOST_NOINLINE static void deserialize(std::string_view str, T &dest)
    {
        if (str.empty())
        {
            throw "expected integer, but got an empty string";
        }
        std::unique_ptr<char[]> mem_holder;
        if (str.back() != '\0')
        {
            char *mem;
            if (str.size() < max_stack_alloc)
            {
                mem = reinterpret_cast<char *>(alloca(str.size() + 1));
            }
            else
            {
                mem_holder = std::make_unique<char[]>(str.size() + 1);
                mem = mem_holder.get();
            }
            memcpy(mem, str.data(), str.size());
            mem[str.size()] = '\0';
            str = std::string_view{ mem, str.size() };
        }

        char *end;
        auto tmp = rdrFunc(str.data(), &end);
        if (errno == ERANGE)
        {
            if (tmp == std::numeric_limits<T>::infinity())
            {
                throw "floating point positive overflow";
            }
            if (tmp == -std::numeric_limits<T>::infinity())
            {
                throw "floating point negative overflow";
            }
            // value underflow
        }
        if (str.data() + str.size() != end)
        {
            throw "couldn't convert a string to a floating point type";
        }
        dest = tmp;
    }

protected:
    constexpr floating_point_serializer() = default;
    ~floating_point_serializer() = default;
};


}

namespace ucmdp
{
    




}
