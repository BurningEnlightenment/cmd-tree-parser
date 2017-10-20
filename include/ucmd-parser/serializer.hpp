// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <tuple>
#include <string>
#include <optional>
#include <type_traits>

namespace ucmdp::detail
{

class cmd_token_stream;

}

namespace ucmdp
{


template< typename T >
struct serialization_traits;


template< typename, typename = std::void_t<> >
struct is_token_consumer : std::false_type
{
};
template< typename T >
struct is_token_consumer< T, std::void_t<decltype(serialization_traits<T>{}.deserialize(std::declval<detail::cmd_token_stream &>(), std::declval<T &>()))> >
    : std::true_type
{
};
template< typename T >
constexpr bool is_token_consumer_v = is_token_consumer<T>::value;

template< typename T >
struct is_optional : std::false_type
{
};
template< typename T >
struct is_optional< std::optional< T > > : std::true_type
{
};
template< typename T >
constexpr bool is_optional_v = is_optional<T>::value;

template< typename T >
struct is_string_view : std::false_type
{
};
template< typename CharT, typename Traits >
struct is_string_view< std::basic_string_view< CharT, Traits > > : std::true_type
{
};
template< typename T >
constexpr bool is_string_view_v = is_string_view<T>::value;


template< >
struct serialization_traits< std::string >
{
    void deserialize(std::string_view in, std::string &out)
    {
        out = in;
    }
};


}
