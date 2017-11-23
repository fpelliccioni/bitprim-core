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
#include <bitcoin/bitcoin/chain/header.hpp>

#include <chrono>
#include <cstddef>
#include <utility>
#include <bitcoin/bitcoin/chain/chain_state.hpp>
#include <bitcoin/bitcoin/chain/compact.hpp>
#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/bitcoin/error.hpp>
#include <bitcoin/bitcoin/math/hash.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

// Use system clock because we require accurate time of day.
using wall_clock = std::chrono::system_clock;

// Constructors.
//-----------------------------------------------------------------------------

header::header(uint32_t version, hash_digest const& previous_block_hash, hash_digest const& merkle, uint32_t timestamp,
               uint32_t bits, uint32_t nonce)
    : version_(version),
      previous_block_hash_(previous_block_hash),
      merkle_(merkle),
      timestamp_(timestamp),
      bits_(bits),
      nonce_(nonce)
{}

header::header()
  : header(0, null_hash, null_hash, 0, 0, 0)
{}

// TODO: implement safe private accessor for conditional cache transfer.
header::header(header const& other)
    : version_(other.version_),
      previous_block_hash_(other.previous_block_hash_),
      merkle_(other.merkle_),
      timestamp_(other.timestamp_),
      bits_(other.bits_),
      nonce_(other.nonce_),
      validation(other.validation)
{}

// TODO: implement safe private accessor for conditional cache transfer.
header::header(header const& other, hash_digest const& hash)
    : header(other)
{
    hash_ = std::make_shared<hash_digest>(hash);    //TODO(fernando): replace shared_ptr with xxx::optional
}

// Operators.
//-----------------------------------------------------------------------------

header& header::operator=(header const& other) {
    // TODO: implement safe private accessor for conditional cache transfer.
    version_ = other.version_;
    previous_block_hash_ = other.previous_block_hash_;
    merkle_ = other.merkle_;
    timestamp_ = other.timestamp_;
    bits_ = other.bits_;
    nonce_ = other.nonce_;
    validation = other.validation;
    return *this;
}

bool operator==(header const& x, header const& y) {
    return (x.version_ == y.version_)
        && (x.previous_block_hash_ == y.previous_block_hash_)
        && (x.merkle_ == y.merkle_)
        && (x.timestamp_ == y.timestamp_)
        && (x.bits_ == y.bits_)
        && (x.nonce_ == y.nonce_);
}

bool operator!=(header const& x, header const& y) {
    return !(x == y);
}

// Deserialization.
//-----------------------------------------------------------------------------

// static
header header::factory_from_data(const data_chunk& data, bool wire) {
    header instance;
    instance.from_data(data, wire);
    return instance;
}

// static
header header::factory_from_data(std::istream& stream, bool wire) {
    header instance;
    instance.from_data(stream, wire);
    return instance;
}

// static
header header::factory_from_data(reader& source, bool wire) {
    header instance;
    instance.from_data(source, wire);
    return instance;
}

bool header::from_data(const data_chunk& data, bool wire) {
    data_source istream(data);
    return from_data(istream, wire);
}

bool header::from_data(std::istream& stream, bool wire) {
    istream_reader source(stream);
    return from_data(source, wire);
}

bool header::from_data(reader& source, bool wire) {
    ////reset();

    version_ = source.read_4_bytes_little_endian();
    previous_block_hash_ = source.read_hash();
    merkle_ = source.read_hash();
    timestamp_ = source.read_4_bytes_little_endian();
    bits_ = source.read_4_bytes_little_endian();
    nonce_ = source.read_4_bytes_little_endian();

    if ( ! wire) {
        validation.median_time_past = source.read_4_bytes_little_endian();
    }

    if ( ! source) {
        reset();
    }

    return source;
}

// protected
void header::reset() {
    version_ = 0;
    previous_block_hash_.fill(0);
    merkle_.fill(0);
    timestamp_ = 0;
    bits_ = 0;
    nonce_ = 0;
    invalidate_cache();
}

bool header::is_valid() const {
    return (version_ != 0) ||
        (previous_block_hash_ != null_hash) ||
        (merkle_ != null_hash) ||
        (timestamp_ != 0) ||
        (bits_ != 0) ||
        (nonce_ != 0);
}

// Serialization.
//-----------------------------------------------------------------------------

data_chunk header::to_data(bool wire) const {
    data_chunk data;
    auto const size = serialized_size(wire);
    data.reserve(size);
    data_sink ostream(data);
    to_data(ostream, wire);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == size);
    return data;
}

void header::to_data(std::ostream& stream, bool wire) const {
    ostream_writer sink(stream);
    to_data(sink, wire);
}

void header::to_data(writer& sink, bool wire) const {
    sink.write_4_bytes_little_endian(version_);
    sink.write_hash(previous_block_hash_);
    sink.write_hash(merkle_);
    sink.write_4_bytes_little_endian(timestamp_);
    sink.write_4_bytes_little_endian(bits_);
    sink.write_4_bytes_little_endian(nonce_);

    if ( ! wire) {
        sink.write_4_bytes_little_endian(validation.median_time_past);
    }
}

// Size.
//-----------------------------------------------------------------------------

// static
size_t header::satoshi_fixed_size() {
    return sizeof(version_)
        + hash_size
        + hash_size
        + sizeof(timestamp_)
        + sizeof(bits_)
        + sizeof(nonce_);
}

size_t header::serialized_size(bool wire) const {
    return satoshi_fixed_size() + (wire ? 0 : sizeof(uint32_t));
}

// Accessors.
//-----------------------------------------------------------------------------

uint32_t header::version() const {
    return version_;
}

void header::set_version(uint32_t value) {
    version_ = value;
    invalidate_cache();
}

hash_digest& header::previous_block_hash() {
    return previous_block_hash_;
}

hash_digest const& header::previous_block_hash() const {
    return previous_block_hash_;
}

void header::set_previous_block_hash(hash_digest const& value) {
    previous_block_hash_ = value;
    invalidate_cache();
}

hash_digest& header::merkle() {
    return merkle_;
}

hash_digest const& header::merkle() const {
    return merkle_;
}

void header::set_merkle(hash_digest const& value) {
    merkle_ = value;
    invalidate_cache();
}

uint32_t header::timestamp() const {
    return timestamp_;
}

void header::set_timestamp(uint32_t value) {
    timestamp_ = value;
    invalidate_cache();
}

uint32_t header::bits() const {
    return bits_;
}

void header::set_bits(uint32_t value) {
    bits_ = value;
    invalidate_cache();
}

uint32_t header::nonce() const {
    return nonce_;
}

void header::set_nonce(uint32_t value) {
    nonce_ = value;
    invalidate_cache();
}

// Cache.
//-----------------------------------------------------------------------------

// protected
void header::invalidate_cache() const {
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (hash_) {
        mutex_.unlock_upgrade_and_lock();
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        hash_.reset();
        //---------------------------------------------------------------------
        mutex_.unlock_and_lock_upgrade();
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////
}

hash_digest header::hash() const {
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if ( ! hash_) {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        hash_ = std::make_shared<hash_digest>(bitcoin_hash(to_data()));
        mutex_.unlock_and_lock_upgrade();
        //---------------------------------------------------------------------
    }

    auto const hash = *hash_;
    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    return hash;
}

#ifdef LITECOIN
hash_digest header::litecoin_proof_of_work_hash() const {
    return litecoin_hash(to_data());
}
#endif //LITECOIN

// Validation helpers.
//-----------------------------------------------------------------------------

/// BUGBUG: bitcoin 32bit unix time: en.wikipedia.org/wiki/Year_2038_problem
bool header::is_valid_timestamp() const {
    using namespace std::chrono;
    static auto const two_hours = seconds(timestamp_future_seconds);
    auto const time = wall_clock::from_time_t(timestamp_);
    auto const future = wall_clock::now() + two_hours;
    return time <= future;
}

// [CheckProofOfWork]
bool header::is_valid_proof_of_work() const {
    // TODO: This should be statically-initialized (optimization).
    static const uint256_t pow_limit(compact{ proof_of_work_limit });

    auto const bits = compact(bits_);

    if (bits.is_overflowed())
        return false;

    uint256_t target(bits);

    // Ensure claimed work is within limits.
    if (target < 1 || target > pow_limit)
        return false;

    // Ensure actual work is at least claimed amount (smaller is more work).
#ifdef LITECOIN
    return to_uint256(litecoin_proof_of_work_hash()) <= target;
#else //LITECOIN
    return to_uint256(hash()) <= target;
#endif //LITECOIN
}

// Validation.
//-----------------------------------------------------------------------------

code header::check() const {
    if (!is_valid_proof_of_work())
        return error::invalid_proof_of_work;

    else if (!is_valid_timestamp())
        return error::futuristic_timestamp;

    else
        return error::success;
}

code header::accept(chain_state const& state) const {
    if (bits_ != state.work_required())
        return error::incorrect_proof_of_work;

    else if (state.is_checkpoint_conflict(hash()))
        return error::checkpoints_failed;

    else if (state.is_under_checkpoint())
        return error::success;

    else if (version_ < state.minimum_version())
        return error::old_version_block;

    else if (timestamp_ <= state.median_time_past())
        return error::timestamp_too_early;

    else
        return error::success;
}

} // namespace chain
} // namespace libbitcoin
