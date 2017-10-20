// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <tuple>
#include <optional>
#include <functional>
#include <type_traits>
#include <string_view>

#include <boost/predef/compiler.h>
#include <boost/config.hpp>

#include "detail/token_stream.hpp"
#include "command_delegate.hpp"
#include "serializer.hpp"
#include "number_serializer.hpp"

namespace ucmdp::detail
{


template< typename Arg >
void parse_arg(cmd_token_stream &cmdTokenStream, Arg &dest)
{
    if constexpr (is_token_consumer_v<Arg>)
    {
        serialization_traits<Arg>{}.deserialize(cmdTokenStream, dest);
    }
    else if constexpr (is_optional_v<Arg>)
    {
        if (cmdTokenStream)
        {
            dest.emplace();

            auto token = cmdTokenStream.next();
            serialization_traits<typename Arg::value_type>{}
                .deserialize(token, *dest);
        }
    }
    else
    {
        if (cmdTokenStream)
        {
            auto token = cmdTokenStream.next();
            serialization_traits<Arg>{}.deserialize(token, dest);
        }
        else
        {
            BOOST_THROW_EXCEPTION(
                not_enough_arguments_error{}
            );
        }
    }
}

template< size_t i, typename... Args >
void parse_args([[maybe_unused]] cmd_token_stream &cmdTokenStream,
                [[maybe_unused]] std::tuple<Args...> &out)
{
    if constexpr (i < sizeof...(Args))
    {
        parse_arg(cmdTokenStream, std::get<i>(out));
        parse_args<i + 1>(cmdTokenStream, out);
    }
}
template< typename... Args >
void parse_args(cmd_token_stream &cmdTokenStream, std::tuple<Args...> &out)
{
    parse_args<0>(cmdTokenStream, out);
}


template< typename T >
using value_type_of = std::remove_const_t<std::remove_reference_t<T>>;


template< typename T >
class command;

template< typename R, typename... Args >
class command<R (Args...)>
{
public:
    // fold expressions would make this far more elegant... fuck you MSVC
    static_assert(std::conjunction_v<std::negation<std::is_pointer<value_type_of<Args>>>...>,
        "Pointer arguments are not allowed for commands!");
    static_assert(std::conjunction_v<std::negation<std::is_array<value_type_of<Args>>>...>,
        "Array arguments are not allowed for commands!");
    static_assert(std::conjunction_v<std::negation<is_string_view<value_type_of<Args>>>...>,
        "basic_string_view arguments are not allowed for commands!");


    using argument_tuple_t = std::tuple<value_type_of<Args>...>;
    using delegate_t = std::function<R(Args...)>;

    explicit command(delegate_t callable)
        : mDelegate(std::move(callable))
    {
    }

    void exec(std::string_view cmd) const
    {
        argument_tuple_t parsedArgs;
        {
            cmd_token_stream cmdTokenStream{ cmd };
            parse_args(cmdTokenStream, parsedArgs);
            if (cmdTokenStream)
            {
                BOOST_THROW_EXCEPTION(
                    too_many_arguments_error{}
                );
            }
        }

        std::apply(mDelegate, parsedArgs);
    }

private:
    const delegate_t mDelegate;
};


}

namespace ucmdp
{


template< typename S >
command_delegate make_command(std::function<S> action)
{
    //static_assert(std::is_same_v<typename std::function<S>::result_type, void>, "commands must not return a value");
    detail::command<S> cmd{ std::move(action) };
    return [cmd](std::string_view params) { cmd.exec(params); };
}

template< typename C >
command_delegate make_command(C func);


}


#if __cpp_deduction_guides >= 201606 // || _MSC_VER >= xxx

template< typename C >
ucmdp::command_delegate ucmdp::make_command(C func)
{
    return make_command(
        std::function{ std::forward<C>(func) }
    );
}

#else

#include "detail/callable_signature_deduction.hpp"

template< typename C >
ucmdp::command_delegate ucmdp::make_command(C func)
{
    return make_command(
        detail::function_type_of<C>{ std::forward<C>(func) }
    );
}

#endif

