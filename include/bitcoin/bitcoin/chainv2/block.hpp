// /**
//  * Copyright (c) 2011-2018 Bitprim developers (see AUTHORS)
//  *
//  * This file is part of Bitprim.
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU Affero General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU Affero General Public License for more details.
//  *
//  * You should have received a copy of the GNU Affero General Public License
//  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
//  */
// #ifndef LIBBITCOIN_CHAINV2_BLOCK_HPP
// #define LIBBITCOIN_CHAINV2_BLOCK_HPP

// #include <cstddef>
// #include <cstdint>
// #include <istream>
// #include <memory>
// #include <string>
// #include <vector>
// #include <boost/optional.hpp>
// #include <bitcoin/bitcoin/chain/chain_state.hpp>
// #include <bitcoin/bitcoin/chainv2/header.hpp>
// #include <bitcoin/bitcoin/chainv2/transaction.hpp>
// #include <bitcoin/bitcoin/define.hpp>
// #include <bitcoin/bitcoin/error.hpp>
// #include <bitcoin/bitcoin/math/hash.hpp>
// #include <bitcoin/bitcoin/utility/asio.hpp>
// #include <bitcoin/bitcoin/utility/data.hpp>
// #include <bitcoin/bitcoin/utility/reader.hpp>
// #include <bitcoin/bitcoin/utility/thread.hpp>
// #include <bitcoin/bitcoin/utility/writer.hpp>

// namespace libbitcoin { namespace chainv2 {

// class BC_API block {
// public:
//     using list = std::vector<block>;
//     using indexes = std::vector<size_t>;

//     // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
//     struct validation {
//         uint64_t originator = 0;
//         code error = error::not_found;
//         chain::chain_state::ptr state = nullptr;

//         // Similate organization and instead just validate the block.
//         bool simulate = false;

//         asio::time_point start_deserialize;
//         asio::time_point end_deserialize;
//         asio::time_point start_check;
//         asio::time_point start_populate;
//         asio::time_point start_accept;
//         asio::time_point start_connect;
//         asio::time_point start_notify;
//         asio::time_point start_pop;
//         asio::time_point start_push;
//         asio::time_point end_push;
//         float cache_efficiency;
//     };

//     // Constructors.
//     //-------------------------------------------------------------------------

//     block();

//     block(block&& other) noexcept;
//     block(block const& other);

//     block(chainv2::header&& header, transaction::list&& transactions);
//     block(chainv2::header const& header, transaction::list const& transactions);

//     // Operators.
//     //-------------------------------------------------------------------------

//     /// This class is move assignable but NOT copy assignable.
//     block& operator=(block&& other) noexcept;
//     block& operator=(block const& other) = delete;

//     bool operator==(block const& other) const;
//     bool operator!=(block const& other) const;

//     // Deserialization.
//     //-------------------------------------------------------------------------

//     static block factory_from_data(const data_chunk& data);
//     static block factory_from_data(std::istream& stream);
//     static block factory_from_data(reader& source);

//     bool from_data(const data_chunk& data);
//     bool from_data(std::istream& stream);
//     bool from_data(reader& source);

//     bool is_valid() const;

//     // Serialization.
//     //-------------------------------------------------------------------------

//     data_chunk to_data() const;
//     void to_data(std::ostream& stream) const;
//     void to_data(writer& sink) const;
//     hash_list to_hashes() const;

//     // Properties (size, accessors, cache).
//     //-------------------------------------------------------------------------

//     size_t serialized_size() const;

//     // deprecated (unsafe)
//     chainv2::header& header();

//     const chainv2::header& header() const;
//     void set_header(const chainv2::header& value);
//     void set_header(chainv2::header&& value);

//     // deprecated (unsafe)
//     transaction::list& transactions();

//     const transaction::list& transactions() const;
//     void set_transactions(const transaction::list& value);
//     void set_transactions(transaction::list&& value);

//     hash_digest hash() const;

//     // Utilities.
//     //-------------------------------------------------------------------------

//     static block genesis_mainnet();
//     static block genesis_testnet();
//     static size_t locator_size(size_t top);
//     static indexes locator_heights(size_t top);

//     // Validation.
//     //-------------------------------------------------------------------------

//     static uint64_t subsidy(size_t height);

//     uint64_t fees() const;
//     uint64_t claim() const;
//     uint64_t reward(size_t height) const;
//     uint256_t proof() const;
//     hash_digest generate_merkle_root() const;
//     size_t signature_operations() const;
//     size_t signature_operations(bool bip16_active) const;
//     size_t total_inputs(bool with_coinbase=true) const;

//     bool is_extra_coinbases() const;
//     bool is_final(size_t height, uint32_t block_time) const;
//     bool is_distinct_transaction_set() const;
//     bool is_valid_coinbase_claim(size_t height) const;
//     bool is_valid_coinbase_script(size_t height) const;
//     bool is_internal_double_spend() const;
//     bool is_valid_merkle_root() const;

//     code check() const;
//     code check_transactions() const;
//     code accept(bool transactions=true) const;
//     code accept(chain::chain_state const& state, bool transactions=true) const;
//     code accept_transactions(chain::chain_state const& state) const;
//     code connect() const;
//     code connect(chain::chain_state const& state) const;
//     code connect_transactions(chain::chain_state const& state) const;

//     // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
//     mutable validation validation;

// protected:
//     void reset();
//     size_t non_coinbase_input_count() const;

// private:
//     chainv2::header header_;
//     transaction::list transactions_;

//     mutable boost::optional<size_t> total_inputs_;
//     mutable upgrade_mutex mutex_;
// };

// }} // namespace libbitcoin::chainv2

// #endif