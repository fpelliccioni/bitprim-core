/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/bitcoin/chainv2/output.hpp>

#include <cstdint>
#include <sstream>
#include <type_traits>

#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>
#include <bitcoin/bitcoin/wallet/payment_address.hpp>

namespace libbitcoin { namespace chainv2 {

static_assert(std::is_move_constructible<output>::value, "std::is_move_constructible<output>::value");
static_assert(std::is_nothrow_move_constructible<output>::value, "std::is_nothrow_move_constructible<output>::value");
static_assert(std::is_move_assignable<output>::value, "std::is_move_assignable<output>::value");
static_assert(std::is_nothrow_move_assignable<output>::value, "std::is_nothrow_move_assignable<output>::value");
static_assert(std::is_copy_constructible<output>::value, "std::is_copy_constructible<output>::value");
static_assert(std::is_copy_assignable<output>::value, "std::is_copy_assignable<output>::value");


// using namespace bc::wallet;
using bc::wallet::payment_address;

// This is a consensus critical value that must be set on reset.
uint64_t const output::not_found = sighash_null_value;

// This is a non-consensus sentinel used to indicate an output is unspent.
uint32_t const output::validation::not_spent = max_uint32;

// Constructors.
//-----------------------------------------------------------------------------

output::output()
    : value_(not_found)
{}

output::output(uint64_t value, chainv2::script&& script)
    : value_(value), script_(std::move(script))
{}

output::output(uint64_t value, chainv2::script const& script)
    : value_(value), script_(script)
{}

// Operators.
//-----------------------------------------------------------------------------

// friend
bool operator==(output const& a, output const& b) {
    return (a.value_ == b.value_) && (a.script_ == b.script_);
}


// friend
bool operator!=(output const& a, output const& b) {
    return !(a == b);
}

// Deserialization.
//-----------------------------------------------------------------------------

output output::factory_from_data(const data_chunk& data, bool wire) {
    output instance;
    instance.from_data(data, wire);
    return instance;
}

output output::factory_from_data(std::istream& stream, bool wire) {
    output instance;
    instance.from_data(stream, wire);
    return instance;
}

output output::factory_from_data(reader& source, bool wire) {
    output instance;
    instance.from_data(source, wire);
    return instance;
}

bool output::from_data(const data_chunk& data, bool wire) {
    data_source istream(data);
    return from_data(istream, wire);
}

bool output::from_data(std::istream& stream, bool wire) {
    istream_reader source(stream);
    return from_data(source, wire);
}

bool output::from_data(reader& source, bool wire) {
    reset();

    if (!wire) {
        validation.spender_height = source.read_4_bytes_little_endian();
    }

    value_ = source.read_8_bytes_little_endian();
    script_.from_data(source, true);

    if (!source) {
        reset();
    }

    return source;
}

// protected
void output::reset() {
    value_ = output::not_found;
    script_.reset();
}

// Empty scripts are valid, validation relies on not_found only.
bool output::is_valid() const {
    return value_ != output::not_found;
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk output::to_data(bool wire) const {
    data_chunk data;
    const auto size = serialized_size(wire);
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream, wire);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void output::to_data(std::ostream& stream, bool wire) const {
    ostream_writer sink(stream);
    to_data(sink, wire);
}

void output::to_data(writer& sink, bool wire) const {
    if ( ! wire) {
        auto height32 = safe_unsigned<uint32_t>(validation.spender_height);
        sink.write_4_bytes_little_endian(height32);
    }

    sink.write_8_bytes_little_endian(value_);
    script_.to_data(sink, true);
}

// Size.
//-----------------------------------------------------------------------------

size_t output::serialized_size(bool wire) const {
    // validation.spender_height is size_t stored as uint32_t.
    return (wire ? 0 : sizeof(uint32_t)) + sizeof(value_) + script_.serialized_size(true);
}

// Accessors.
//-----------------------------------------------------------------------------

uint64_t output::value() const {
    return value_;
}

void output::set_value(uint64_t value) {
    value_ = value;
}

chainv2::script& output::script() {
    return script_;
}

chainv2::script const& output::script() const {
    return script_;
}

void output::set_script(chainv2::script const& value) {
    script_ = value;
    // invalidate_cache();
}

void output::set_script(chainv2::script&& value) {
    script_ = std::move(value);
    // invalidate_cache();
}

bool output::is_dust(uint64_t minimum_output_value) const {
    // If provably unspendable it does not expand the unspent output set.
    return value_ < minimum_output_value && !script_.is_unspendable();
}

// // protected
// void output::invalidate_cache() const {
//     ///////////////////////////////////////////////////////////////////////////
//     // Critical Section
//     mutex_.lock_upgrade();

//     if (address_) {
//         mutex_.unlock_upgrade_and_lock();
//         //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//         address_.reset();
//         //---------------------------------------------------------------------
//         mutex_.unlock_and_lock_upgrade();
//     }

//     mutex_.unlock_upgrade();
//     ///////////////////////////////////////////////////////////////////////////
// }

payment_address output::address(bool testnet /* = false */) const {
    // TODO(libbitcoin): limit this to output patterns.
    if (! testnet) {
        return payment_address::extract(script_);
    } 

    return payment_address::extract(script_, payment_address::testnet_p2kh, payment_address::testnet_p2sh);
}

// Validation helpers.
//-----------------------------------------------------------------------------

size_t output::signature_operations() const {
    return script_.sigops(false);
}

}} // namespace libbitcoin::chainv2