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

add_mingw_unicode_flags()

set(_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

add_executable(cmd_parser-tests
    cmd_parser-tests.cpp
    boost-unit-test.hpp
	
	misc-tests.cpp
	
	# src files for vs ide support
	"${_INCLUDE_DIR}/command_parser.hpp"
)
target_link_libraries(cmd_parser-tests
    PUBLIC
		cmd_parser
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
target_include_directories(cmd_parser-tests
	PUBLIC
		${Boost_INCLUDE_DIRS}
)

add_test(NAME cmd_parser-tests
    COMMAND cmd_parser-tests
)
