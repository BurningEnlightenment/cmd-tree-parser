// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#include <ucmd-parser/command_parser.hpp>
#include "boost-unit-test.hpp"

BOOST_AUTO_TEST_SUITE(misc_tests)

void blabluc(float, int, std::string) noexcept
{
    
}

BOOST_AUTO_TEST_CASE(simple_compile_test)
{
    using namespace ucmdp;
    command_tree tree;

    make_command(blabluc);
    make_command(std::hash<int>{});
}


BOOST_AUTO_TEST_SUITE_END()
