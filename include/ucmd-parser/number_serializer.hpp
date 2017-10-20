// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <memory.h>
#include <malloc.h>

#include <memory>
#include <limits>
#include <type_traits>
#include <stdexcept>
#include <string_view>

#include <boost/config.hpp>

#include "exceptions.hpp"
#include "serializer.hpp"

namespace ucmdp::detail
{


template< typename T, T(*rdrFunc)(const char *, char **, int) >
struct integer_serializer_base
{
    static_assert(std::is_integral_v<T>,
        "integer_serializer_base is only meaningful for integer types");

    using argument_type = T;

    static constexpr std::size_t max_stack_alloc = 72;

    // no inline because of alloca usage
    BOOST_NOINLINE void deserialize(std::string_view str, T &dest) const
    {
        if (str.empty())
        {
            BOOST_THROW_EXCEPTION(
                empty_argument_error{}
            );
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
                BOOST_THROW_EXCEPTION(
                    integer_overflow_serialization_error{}
                );
            }
            if (tmp == std::numeric_limits<T>::min())
            {
                BOOST_THROW_EXCEPTION(
                    integer_underflow_serialization_error{}
                );
            }
            BOOST_THROW_EXCEPTION(
                integer_serialization_error{}
            );
        }
        if (str.data() + str.size() != end)
        {
            BOOST_THROW_EXCEPTION(
                invalid_integer_error{}
            );
        }
        dest = tmp;
    }

protected:
    constexpr explicit integer_serializer_base(int base = 0)
        : base(base)
    {
        if (base != 0 && (base < 2 || base > 36))
        {
            BOOST_THROW_EXCEPTION(
                std::invalid_argument("base is neither 0 nor in the interval [2, 36]")
            );
        }
    }
    ~integer_serializer_base() = default;

private:
    const int base;
};

using longlong_serializer = integer_serializer_base<long long, strtoll>;
using long_serializer = integer_serializer_base<long, strtol>;
using ulonglong_serializer = integer_serializer_base<unsigned long long, strtoull>;
using ulong_serializer = integer_serializer_base<unsigned long, strtoul>;

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

    constexpr explicit small_integer_serializer(int base = 0)
        : base_t(base)
    {
    }

    void deserialize(std::string_view str, T &dest) const
    {
        typename base_t::argument_type tmp;
        base_t::deserialize(str, tmp);
        if (tmp > std::numeric_limits<T>::max())
        {
            BOOST_THROW_EXCEPTION(
                integer_overflow_serialization_error{}
            );
        }
        if (std::is_signed<T>::value
            && tmp < std::numeric_limits<T>::min())
        {
            BOOST_THROW_EXCEPTION(
                integer_underflow_serialization_error{}
            );
        }
        dest = static_cast<T>(tmp);
    }
};


template< typename T, T(*rdrFunc)(const char *, char**) >
struct floating_point_serializer_base
{
    static_assert(std::is_floating_point_v<T>,
        "floating_point_serializer_base is only meaningful for floating point types");

    static constexpr std::size_t max_stack_alloc = 72;

    BOOST_NOINLINE static void deserialize(std::string_view str, T &dest)
    {
        if (str.empty())
        {
            BOOST_THROW_EXCEPTION(
                empty_argument_error{}
            );
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
                BOOST_THROW_EXCEPTION(
                    floating_point_overflow_serialization_error{}
                );
            }
            if (tmp == -std::numeric_limits<T>::infinity())
            {
                BOOST_THROW_EXCEPTION(
                    floating_point_underflow_serialization_error{}
                );
            }
            // value underflow
        }
        if (str.data() + str.size() != end)
        {
            BOOST_THROW_EXCEPTION(
                invalid_floating_point_error{}
            );
        }
        dest = tmp;
    }

protected:
    constexpr floating_point_serializer_base() = default;
    ~floating_point_serializer_base() = default;
};


}

namespace ucmdp
{

template< >
struct serialization_traits<long long>
    : private detail::longlong_serializer
{
    using detail::longlong_serializer::deserialize;
};
template< >
struct serialization_traits<unsigned long long>
    : private detail::ulonglong_serializer
{
    using detail::ulonglong_serializer::deserialize;
};

template< >
struct serialization_traits<long>
    : private detail::long_serializer
{
    using detail::long_serializer::deserialize;
};
template< >
struct serialization_traits<unsigned long>
    : private detail::ulong_serializer
{
    using detail::ulong_serializer::deserialize;
};

template< >
struct serialization_traits<int>
    : private detail::small_integer_serializer<int>
{
    using detail::small_integer_serializer<int>::deserialize;
};
template< >
struct serialization_traits<unsigned int>
    : private detail::small_integer_serializer<unsigned int>
{
    using detail::small_integer_serializer<unsigned int>::deserialize;
};

template< >
struct serialization_traits<std::byte>
{
private:
    using number_type = std::underlying_type_t<std::byte>;
    using impl_type = detail::small_integer_serializer<number_type>;

public:
    void deserialize(std::string_view str, std::byte &out)
    {
        number_type tmp;
        if (str.size() > 2 && str[0] == '0' && str[1] == 'b')
        {
            mBinSerializer.deserialize(str, tmp);
        }
        else
        {
            mStdSerializer.deserialize(str, tmp);
        }
        out = std::byte{tmp};
    }

private:
    impl_type mStdSerializer = impl_type(0);
    impl_type mBinSerializer = impl_type(2);
};


template< >
struct serialization_traits<long double>
    : private detail::floating_point_serializer_base<long double, &strtold>
{
    using detail::floating_point_serializer_base<long double, &strtold>::deserialize;
};

template< >
struct serialization_traits<double>
    : private detail::floating_point_serializer_base<double, &strtod>
{
    using detail::floating_point_serializer_base<double, &strtod>::deserialize;
};

template< >
struct serialization_traits<float>
    : private detail::floating_point_serializer_base<float, &strtof>
{
    using detail::floating_point_serializer_base<float, &strtof>::deserialize;
};

}
