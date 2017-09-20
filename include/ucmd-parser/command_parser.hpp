// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <tuple>
#include <memory>
#include <functional>
#include <string_view>  

#include <boost/predef/compiler.h>
#include <boost/config.hpp>   
#include <boost/tokenizer.hpp>
#include <boost/container/flat_map.hpp>

#include "serializer.hpp"
#include "number_serializer.hpp"

namespace ucmdp::detail
{


using escaped_cmd_seperator = boost::escaped_list_separator<char>;
inline escaped_cmd_seperator make_escaped_cmd_seperator()
{
    return escaped_cmd_seperator { '\\', ' ', '"' };
}

class cmd_token_stream
{
public:
    explicit cmd_token_stream(std::string_view sequence)
        : tokenize(make_escaped_cmd_seperator())
        , mSequence(sequence)
    {
        mNextPos = mSequence.cbegin();
    }
    cmd_token_stream(const cmd_token_stream &) = delete;

    explicit operator bool() const
    {
        return mNextPos != mSequence.end();
    }

    std::string next()
    {
        std::string token;
        if (!tokenize(mNextPos, mSequence.cend(), token))
        {
            throw "not enough input";
        }
        return token;
    }
    std::string_view remaining() const
    {
        return mSequence.substr(mNextPos - mSequence.cbegin());
    }

private:
    escaped_cmd_seperator tokenize;
    std::string_view mSequence;
    std::string_view::const_iterator mNextPos;
};

template< bool last, typename Arg >
Arg parse_arg([[maybe_unused]] cmd_token_stream &cmdTokenStream)
{
    
    return Arg{};
}

template< typename ParsedArgsTuple >
ParsedArgsTuple parse_args(ParsedArgsTuple parsedArgs, cmd_token_stream &cmdTokenStream)
{
    if (cmdTokenStream)
    {
        //TODO: exception stuff
        throw "more input than expected";
    }
    return parsedArgs;
}
template< typename ParsedArgsTuple, typename CurrentArg, typename... MoreArgs >
auto parse_args(ParsedArgsTuple prev, cmd_token_stream &cmdTokenStream)
{
    std::string tokenOwner;
    std::string_view token;

    if constexpr (sizeof...(MoreArgs) == 0)
    {
    }
    else
    {
        token = tokenOwner = cmdTokenStream.next();
    }

    auto parsed = parse_arg<sizeof...(MoreArgs) == 0, CurrentArg>(cmdTokenStream);
    auto argsTuple = std::tuple_cat(std::move(prev), std::tuple<CurrentArg>{ std::move(parsed) });
    return parse_args<decltype(argsTuple), MoreArgs...>(std::move(argsTuple), cmdTokenStream);
}
template< typename... Args >
std::tuple<Args...> parse_args(cmd_token_stream &cmdTokenStream)
{
    return parse_args<std::tuple<>, Args...>(std::tuple<>{}, cmdTokenStream);
}


template< typename T >
class command;

template< typename R, typename... Args >
class command<R (Args...)>
{
public:
    using argument_tuple_t = std::tuple<Args...>;
    using delegate_t = std::function<R(Args...)>;

    explicit command(delegate_t callable)
        : mDelegate(std::move(callable))
    {
    }

    void exec(std::string_view cmd) const
    {
        cmd_token_stream cmdTokenStream{ cmd };

        auto parsedArgs = parse_args<Args...>(cmdTokenStream);

        std::apply(mDelegate, parsedArgs);
    }

private:
    const delegate_t mDelegate;
};


template< typename, class = std::void_t<> >
struct is_functor : std::false_type
{
};

template< typename T >
struct is_functor<T, std::void_t<decltype(&T::operator())> > : std::true_type
{
};

template< typename T >
constexpr bool is_functor_v = is_functor<T>::value;


template< typename C >
struct functor_deduction_helper;

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...)>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) volatile>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const volatile>
{
    using function_type = std::function<R(Args...)>;
};

#if __cpp_noexcept_function_type >= 201510 // || _MSC_VER >= xxx

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) volatile noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename T, typename... Args >
struct functor_deduction_helper<R(T::*)(Args...) const volatile noexcept>
{
    using function_type = std::function<R(Args...)>;
};

#endif


template< typename T >
struct callable_signature_deducer
{
    static_assert(is_functor_v<T>, "cannot deduce the signature of non function types");
    using function_type = typename functor_deduction_helper<decltype(&T::operator())>::function_type;
};

template< typename R, typename... Args >
struct callable_signature_deducer<R (&)(Args...)>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename... Args >
struct callable_signature_deducer<R (*)(Args...)>
{
    using function_type = std::function<R(Args...)>;
};

#if __cpp_noexcept_function_type >= 201510 // || _MSC_VER >= xxx

template< typename R, typename... Args >
struct callable_signature_deducer<R(&)(Args...) noexcept>
{
    using function_type = std::function<R(Args...)>;
};

template< typename R, typename... Args >
struct callable_signature_deducer<R(*)(Args...) noexcept>
{
    using function_type = std::function<R(Args...)>;
};

#endif


template< typename C >
using function_type_of = typename callable_signature_deducer<C>::function_type;


}

namespace ucmdp
{


using command_delegate = std::function<void(std::string_view)>;


class command_tree
{
public:
    class node
    {
    public:
        using map = boost::container::flat_map<std::string, node>;

        explicit node() = default;
        node(command_delegate action);
        node(map childs, command_delegate action = command_delegate{});

        void operator()(detail::cmd_token_stream &params) const;

    private:
        map mChilds;
        command_delegate mAction;
    };

    explicit command_tree() = default;
    explicit command_tree(node rootNode);
    explicit command_tree(node::map commands);
    command_tree(const command_tree &) = delete;
    command_tree & operator=(const command_tree &) = delete;


    

private:
    node mCommandTreeRoot;
};

inline command_tree::node::node(command_delegate action)
    : mChilds()
    , mAction(std::move(action))
{
}

inline command_tree::node::node(map childs, command_delegate action)
    : mChilds(std::move(childs))
    , mAction(std::move(action))
{
}

inline void command_tree::node::operator()(detail::cmd_token_stream &params) const
{
    auto fullParamStr = params.remaining();
    auto currentParam = params.next();
    if (auto childCmdIter = mChilds.find(currentParam);
            childCmdIter != mChilds.end())
    {
        childCmdIter->second(params);
    }
    else if (mAction)
    {
        mAction(fullParamStr);
    }
    else
    {
        throw "unknown command";
    }
}

inline command_tree::command_tree(node rootNode)
    : mCommandTreeRoot(std::move(rootNode))
{
}

inline command_tree::command_tree(node::map commands)
    : mCommandTreeRoot(std::move(commands))
{
}

template< typename S >
command_delegate make_command(std::function<S> action)
{
    //static_assert(std::is_same_v<typename std::function<S>::result_type, void>, "commands must not return a value");
    detail::command<S> cmd{ std::move(action) };
    return [cmd](std::string_view params) { cmd.exec(params); };
}

#if 0 && __cpp_deduction_guides >= 201606 // || _MSC_VER >= xxx

template< typename C >
command_delegate make_command(C func)
{
    std::function action{ std::forward<C>(func) };
    return make_command(action);
}

#else

template< typename C >
command_delegate make_command(C func)
{
    return make_command(
        detail::function_type_of<C>{ std::forward<C>(func) }
    );
}

#endif


}
