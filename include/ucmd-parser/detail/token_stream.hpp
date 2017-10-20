// Copyright 2017 Henrik Steffen Gaßmann
//
// Licensed under the MIT License.
// See LICENSE file in the project root for full license information.
//
#pragma once

#include <string>
#include <string_view>

#include <boost/tokenizer.hpp>

#include "../exceptions.hpp"

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

    explicit operator bool() const
    {
        return mNextPos != mSequence.end();
    }

    std::string next()
    {
        std::string token;
        if (!tokenize(mNextPos, mSequence.cend(), token))
        {
            BOOST_THROW_EXCEPTION(
                end_of_token_stream_error{}
            );
        }
        return token;
    }

    std::string_view sequence() const
    {
        return mSequence;
    }
    std::string_view consumed() const
    {
        return mSequence.substr(0, mNextPos - mSequence.cbegin());
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


}
