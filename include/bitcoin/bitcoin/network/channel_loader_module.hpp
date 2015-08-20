/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_CHANNEL_LOADER_MODULE_HPP
#define LIBBITCOIN_CHANNEL_LOADER_MODULE_HPP

#include <functional>
#include <string>
#include <system_error>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/error.hpp>
#include <bitcoin/bitcoin/utility/data.hpp>

namespace libbitcoin {
namespace network {

class BC_API channel_loader_module_base
{
public:
    virtual ~channel_loader_module_base() {}
    virtual std::error_code attempt_load(std::istream& stream) const = 0;
    virtual const std::string lookup_symbol() const = 0;
};

template <typename Message>
class channel_loader_module
  : public channel_loader_module_base
{
public:
    typedef std::function<void (const std::error_code&, const Message&)>
        load_handler;

    channel_loader_module(load_handler handle_load)
      : handle_load_(handle_load)
    {
    }

    /// This class is not copyable.
    channel_loader_module(const channel_loader_module&) = delete;
    void operator=(const channel_loader_module&) = delete;

    std::error_code attempt_load(std::istream& stream) const
    {
        std::error_code status = bc::error::success;
        Message result;

        if (!result.from_data(stream))
        {
            status = bc::error::bad_stream;
            result = Message();
        }

        handle_load_(status, result);

        return status;
    }

    const std::string lookup_symbol() const
    {
        return Message::satoshi_command;
    }

private:

    load_handler handle_load_;
};

} // namespace network
} // namespace libbitcoin

#endif

