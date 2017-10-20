# Written in 2017 by Henrik Steffen Gaßmann <henrik@gassmann.onl>
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to the
# public domain worldwide. This software is distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software. If not, see
#
#     http://creativecommons.org/publicdomain/zero/1.0/
#
########################################################################

set(_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/ucmd-parser")

add_executable(cmd_parser-tests
    cmd_parser-tests.cpp
    boost-unit-test.hpp
	
    misc-tests.cpp
    command-tests.cpp
    command_tree-tests.cpp
	
    # src files for vs ide support
    "${_INCLUDE_DIR}/all.hpp"

    "${_INCLUDE_DIR}/command_delegate.hpp"
    "${_INCLUDE_DIR}/command.hpp"
    "${_INCLUDE_DIR}/command_tree.hpp"
    
    "${_INCLUDE_DIR}/serializer.hpp"
    "${_INCLUDE_DIR}/number_serializer.hpp"

    "${_INCLUDE_DIR}/detail/callable_signature_deduction.hpp"
    "${_INCLUDE_DIR}/detail/token_stream.hpp"
)
target_link_libraries(cmd_parser-tests
    PUBLIC
		cmd-tree-parser
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
target_include_directories(cmd_parser-tests
	PUBLIC
		${Boost_INCLUDE_DIRS}
)

add_test(NAME cmd_parser-tests
    COMMAND cmd_parser-tests
)
