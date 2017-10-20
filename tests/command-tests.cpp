// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#include <ucmd-parser/command.hpp>
#include "boost-unit-test.hpp"

using namespace ucmdp;
using namespace ucmdp::detail;


BOOST_AUTO_TEST_SUITE(command_tests)


BOOST_AUTO_TEST_CASE(simple_cmd)
{
    bool called = false;

    command<void()> cmd([&called]() { called = true; });
    cmd.exec("");
    BOOST_TEST(called);
}

BOOST_AUTO_TEST_CASE(simple_cmd_too_many_args)
{
    bool called = false;

    command<void()> cmd([&called]() { called = true; });
    BOOST_CHECK_THROW(cmd.exec("x"), too_many_arguments_error);
}

BOOST_AUTO_TEST_CASE(cmd_with_int_arg)
{
    int val = 35468;
    std::string serVal = std::to_string(val);

    command<void(int)> cmd([&val](int argval) { BOOST_TEST(argval == val); });
    BOOST_CHECK_NO_THROW(cmd.exec(serVal));
}


BOOST_AUTO_TEST_SUITE_END()
