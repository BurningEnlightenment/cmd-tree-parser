// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <functional>
#include <string_view>


namespace ucmdp
{


using command_delegate = std::function<void(std::string_view)>;


}
