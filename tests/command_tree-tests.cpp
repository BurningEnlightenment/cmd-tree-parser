// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#include <ucmd-parser/command_tree.hpp>
#include "boost-unit-test.hpp"

using namespace ucmdp;

BOOST_AUTO_TEST_SUITE(command_tree_tests)


BOOST_AUTO_TEST_CASE(ctor)
{
    command_tree tr;
    command_tree::node n;
}

BOOST_AUTO_TEST_CASE(ctor_simple_insert)
{
    bool simpleCalled = false;
    command_tree cmds {
        { "simple", [&simpleCalled](std::string_view){ simpleCalled = true; } }
    };

    cmds("simple");
    BOOST_TEST(simpleCalled);
}

BOOST_AUTO_TEST_CASE(tree_simple_insert)
{
    bool simpleCalled = false;
    command_tree cmds;

    cmds.insert("simple", [&simpleCalled](std::string_view){ simpleCalled = true; });

    cmds("simple");
    BOOST_TEST(simpleCalled);
}

BOOST_AUTO_TEST_CASE(tree_insert)
{
    bool simpleCalled = false;
    bool secondCalled = false;
    bool thirdCalled = false;
    bool complexCalled = false;
    command_tree cmds;

    cmds.insert("simple", [&simpleCalled](std::string_view){ simpleCalled = true; });
    cmds.insert("x y z", [&secondCalled](std::string_view){ secondCalled = true; });
    cmds.insert("x y y", [&thirdCalled](std::string_view){ thirdCalled = true; });
    cmds.insert("x y", [&complexCalled](std::string_view){ complexCalled = true; });

    cmds("simple");
    BOOST_TEST(simpleCalled);

    cmds("x y z");
    BOOST_TEST(secondCalled);

    cmds("x y y");
    BOOST_TEST(thirdCalled);

    cmds("x y");
    BOOST_TEST(complexCalled);

    complexCalled = false;
    cmds("x y fasdf dasf adf");
    BOOST_TEST(complexCalled);
}

BOOST_AUTO_TEST_CASE(cmd_argument)
{
    command_tree cmds {
        { "xda yd z", [](std::string_view){ BOOST_TEST(false); } },
        { "xda yd y", [](std::string_view){ BOOST_TEST(false); } },
        { "xda yd", [](std::string_view str){ BOOST_TEST(str == "cmplx 0 55 cmd arguments"); } }
    };

    cmds("xda yd cmplx 0 55 cmd arguments");
}


BOOST_AUTO_TEST_SUITE_END()
