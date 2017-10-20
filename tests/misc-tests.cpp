// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#include <ucmd-parser/all.hpp>
#include "boost-unit-test.hpp"

struct test_token_consumer
{
};
namespace ucmdp
{
    template< >
    struct serialization_traits< test_token_consumer >
    {
        static void deserialize(detail::cmd_token_stream &, test_token_consumer &)
        {

        }
    };
}

BOOST_AUTO_TEST_SUITE(misc_tests)

static_assert(ucmdp::is_token_consumer_v<test_token_consumer>);
static_assert(ucmdp::is_optional_v<std::optional<int>>);
static_assert(ucmdp::is_string_view_v<std::string_view>);

void blabluc(long long, test_token_consumer) noexcept
{

}

BOOST_AUTO_TEST_CASE(simple_compile_test)
{
    using namespace ucmdp;
    command_tree tree {
        { "blablub", make_command(blabluc) },
        //{ "hash int", make_command(std::hash<int>{})},
        //{ "call del1", make_command([](int) { }) },
        { "call del2", make_command([](std::string) { }) }
    };

    make_command(blabluc);
    //make_command(std::hash<int>{});
}


BOOST_AUTO_TEST_SUITE_END()
