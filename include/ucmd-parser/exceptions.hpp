// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <exception>
#include <string_view>

#include <boost/exception/all.hpp>

namespace ucmdp
{

using last_token_info = boost::error_info<struct last_token_info_tag, std::string>;
using command_part_info = boost::error_info<struct command_part_info_tag, std::string>;
using arg_part_info = boost::error_info<struct arg_part_info_tag, std::string>;

class cmd_exception
    : public virtual std::exception
    , public virtual boost::exception
{
public:
};

class command_not_found_error
    : public virtual cmd_exception
{
};

class command_argument_serialization_error
    : public virtual cmd_exception
{
};

class empty_argument_error
    : public virtual command_argument_serialization_error
{
};

class integer_serialization_error
    : public virtual command_argument_serialization_error
{
};

class integer_overflow_serialization_error
    : public virtual integer_serialization_error
{
};

class integer_underflow_serialization_error
    : public virtual integer_serialization_error
{
};

class invalid_integer_error
    : public virtual integer_serialization_error
{
};

class floating_point_serialization_error
    : public virtual command_argument_serialization_error
{
};

class floating_point_overflow_serialization_error
    : public virtual floating_point_serialization_error
{
};

class floating_point_underflow_serialization_error
    : public virtual floating_point_serialization_error
{
};

class invalid_floating_point_error
    : public virtual floating_point_serialization_error
{
};


class too_many_arguments_error
    : public virtual command_not_found_error
    , public virtual command_argument_serialization_error
{
};

class not_enough_arguments_error
    : public virtual command_not_found_error
    , public virtual command_argument_serialization_error
{
};

class token_stream_error
    : public virtual cmd_exception
{
};

class end_of_token_stream_error
    : public virtual token_stream_error
{
};


}
