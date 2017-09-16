// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <tuple>
#include <functional>
#include <string_view>

namespace reactionwheel
{

namespace cmd::detail
{


template< typename Arg >
std::tuple< std::tuple< Arg >, std::string_view > parse_arg(std::string_view cmd)
{
    return { { Arg{} }, cmd };
}

template< typename ParsedArgsTuple >
ParsedArgsTuple parse_args(ParsedArgsTuple parsedArgs, std::string_view cmd)
{
    if (cmd.size())
    {
        //TODO: exception stuff
        throw "more input than expected";
    }
    return parsedArgs;
}
template< typename ParsedArgsTuple, typename CurrentArg, typename... MoreArgs >
auto parse_args(ParsedArgsTuple prev, std::string_view cmd)
{
    auto [parsed, remaining] = parse_arg<CurrentArg>(cmd);
    auto argsTuple = std::tuple_cat(prev, parsed);
    return parse_args<decltype(argsTuple), MoreArgs...>(argsTuple, remaining);
}
template< typename... Args >
std::tuple<Args...> parse_args(std::string_view cmd)
{
    return parse_args<std::tuple<>>(std::tuple<>{}, cmd);
}


template< typename T >
class command;

template< typename R, typename... Args >
class command<R (Args...)>
{
public:
    using argument_tuple_t = std::tuple<Args...>;
    using delegate_t = std::function<R(Args...)>;

    template< typename C >
    explicit command(C callable)
        : mDelegate(std::move(callable))
    {
    }

    R exec(std::string_view cmd)
    {
        auto parsedArgs = parse_args<Args...>(cmd);
        return std::apply(mDelegate, parsedArgs);
    }

private:
    delegate_t mDelegate;
};

}


}
