/**
 * Copyright (c) 2011-2018 Bitprim developers (see AUTHORS)
 *
 * This file is part of Bitprim.
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
#ifndef LIBBITCOIN_CHAINV2_OUTPUT_POINT_HPP_
#define LIBBITCOIN_CHAINV2_OUTPUT_POINT_HPP_

#include <cstddef>
#include <cstdint>
#include <vector>

#include <bitcoin/bitcoin/chain/output_point.hpp>

#include <bitcoin/bitcoin/chainv2/output.hpp>
#include <bitcoin/bitcoin/chainv2/point.hpp>
#include <bitcoin/bitcoin/chainv2/script.hpp>
#include <bitcoin/bitcoin/define.hpp>

namespace libbitcoin { namespace chainv2 {

class BC_API output_point
    : public point
{
public:

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    struct validation_type {
        /// An output is spent if a valid transaction has a valid claim on it.
        /// When validating blocks only long chain blocks can have a claim.
        /// When validating memory pool tx another mempool tx can have a claim.
        bool spent = false;

        /// A spend is confirmed if spender is in long chain (not memory pool).
        bool confirmed = false;

        /// The previous output is a coinbase (must verify spender maturity).
        bool coinbase = false;

        /// Prevout height is used for coinbase maturity and relative lock time.
        size_t height = 0;

        /// Median time past is used for relative lock time.
        uint32_t median_time_past = 0;

        /// The output cache contains the output referenced by the input point.
        /// If the cache.value is not_found (default) the output is not found.
        output cache = output{};
    };

    // Constructors.
    //-------------------------------------------------------------------------

    output_point() = default;

    explicit
    output_point(point const& value);

    output_point(hash_digest const& hash, uint32_t index);

    // output_point(output_point&& other) noexcept;
    // output_point(output_point&& other) = default;
    // output_point(output_point const& other) = default;


    // Operators.
    //-------------------------------------------------------------------------
    // This class is move assignable and copy assignable.

    // output_point& operator=(output_point&& other) noexcept;
    // output_point& operator=(output_point const& other) = default;

    output_point& operator=(point const& other);


    friend
    bool operator==(output_point const& a, output_point const& b);

    friend
    bool operator!=(output_point const& a, output_point const& b);

    friend
    bool operator==(output_point const& a, point const& b);

    friend
    bool operator!=(output_point const& a, point const& b);

    friend
    bool operator==(point const& a, output_point const& b);

    friend
    bool operator!=(point const& a, output_point const& b);


    // Deserialization.
    //-------------------------------------------------------------------------

    static 
    output_point factory_from_data(data_chunk const& data, bool wire=true);
    
    static 
    output_point factory_from_data(std::istream& stream, bool wire=true);

    static 
    output_point factory_from_data(reader& source, bool wire=true);

    // Validation.
    //-------------------------------------------------------------------------

    /// True if cached previous output is mature enough to spend from height.
    bool is_mature(size_t height) const;

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    mutable validation_type validation;

protected:
    // So that input may call reset from its own.
    friend class input;
};

}} // namespace libbitcoin::chainv2

#endif // LIBBITCOIN_CHAINV2_OUTPUT_POINT_HPP_