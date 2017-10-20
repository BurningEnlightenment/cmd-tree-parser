// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <tuple>
#include <string_view>
#include <initializer_list>

#include <boost/predef.h>
#include <boost/container/flat_map.hpp>

#include "exceptions.hpp"
#include "detail/token_stream.hpp"
#include "command_delegate.hpp"

namespace ucmdp
{


class command_tree
{
public:
    class node
    {
    public:
        using map = boost::container::flat_map<std::string, node>;

        explicit node() = default;
        //node(command_delegate action);
        //node(map childs, command_delegate action = command_delegate{});

        void insert(detail::cmd_token_stream &nameTokenStream, command_delegate action);
        void operator()(detail::cmd_token_stream &params) const;

    private:
        void exec(std::string_view args) const;

        map mChilds;
        command_delegate mAction;
    };

    using name_cmd_tuple = std::tuple<std::string_view, command_delegate>;
    using initializer_list = std::initializer_list<name_cmd_tuple>;

    explicit command_tree() = default;
    //explicit command_tree(node rootNode);
    command_tree(initializer_list commands);
    command_tree(const command_tree &) = delete;
    command_tree & operator=(const command_tree &) = delete;

    void insert(std::string_view cmd, command_delegate action);
    void operator()(std::string_view cmd) const;

private:
    static node swallow(initializer_list commands);
    static void insert_cmd(node &root, std::string_view fullName, command_delegate action);

    node mCommandTreeRoot;
};

/*
inline command_tree::node::node(command_delegate action)
    : mChilds()
    , mAction(std::move(action))
{
}
*/

/*
inline command_tree::node::node(map childs, command_delegate action)
    : mChilds(std::move(childs))
    , mAction(std::move(action))
{
}
*/

inline void command_tree::node::insert(detail::cmd_token_stream &nameTokenStream, command_delegate action)
{
    if (!nameTokenStream)
    {
        mAction = std::move(action);
    }
    else
    {
        auto next = nameTokenStream.next();
        auto &child = mChilds[std::move(next)];
        child.insert(nameTokenStream, std::move(action));
    }
}

inline void command_tree::node::exec(std::string_view args) const
{
    if (mAction)
    {
        mAction(args);
    }
    else
    {
        BOOST_THROW_EXCEPTION(
            command_not_found_error{}
        );
    }
}

inline void command_tree::node::operator()(detail::cmd_token_stream &params) const
{
    std::string_view cmd_part = params.consumed();
    try
    {
        if (params)
        {
            auto fullParamStr = params.remaining();
            auto currentParam = params.next();
            if (auto childCmdIter = mChilds.find(currentParam);
                    childCmdIter != mChilds.end())
            {
                childCmdIter->second(params);
            }
            else
            {
                try
                {
                    exec(fullParamStr);
                }
                catch (boost::exception &exc)
                {
                    exc << arg_part_info(std::string{fullParamStr});
                    exc << last_token_info{ std::move(currentParam) };
                    throw;
                }
            }
        }
        else
        {
            exec("");
        }
    }
    catch (boost::exception &exc)
    {
        exc << command_part_info(std::string{cmd_part});
        throw;
    }
}

/*
inline command_tree::command_tree(node rootNode)
    : mCommandTreeRoot(std::move(rootNode))
{
}
*/

inline command_tree::command_tree(initializer_list cmds)
    : mCommandTreeRoot(swallow(cmds))
{
}

inline void command_tree::insert_cmd(node &root, std::string_view fullName, command_delegate action)
{
    detail::cmd_token_stream nameTokenStream { fullName };

    root.insert(nameTokenStream, action);
}

inline auto command_tree::swallow(initializer_list cmds)
    -> node
{
    node root;
#if BOOST_COMP_MSVC <= BOOST_VERSION_NUMBER(19,11,0)
    for (auto& t : cmds)
    {
        insert_cmd(root, std::move(std::get<0>(t)), std::move(std::get<1>(t)));
    }
#else
    for (auto& [cmdName, cmdDelegate] : cmds)
    {
        insert_cmd(root, std::move(cmdName), std::move(cmdDelegate));
    }
#endif
    return root;
}

inline void command_tree::insert(std::string_view cmd, command_delegate action)
{
    insert_cmd(mCommandTreeRoot, cmd, action);
}

inline void command_tree::operator()(std::string_view cmd) const
{
    detail::cmd_token_stream cmdTokenStream { cmd };
    mCommandTreeRoot(cmdTokenStream);
}


}
