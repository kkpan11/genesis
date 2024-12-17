/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2018-2024 Lucas Czech

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contact:
    Lucas Czech <lucas.czech@sund.ku.dk>
    University of Copenhagen, Globe Institute, Section for GeoGenetics
    Oster Voldgade 5-7, 1350 Copenhagen K, Denmark
*/

/**
 * @brief
 *
 * @file
 * @ingroup sequence
 */

#include "genesis/sequence/kmer/canonical_encoding.hpp"

namespace genesis {
namespace sequence {

// ================================================================================================
//     Definitions
// ================================================================================================

// Need out-of-line definition in C++11... Just to make the compiler happy.

constexpr std::array<uint64_t, 16> MinimalCanonicalEncoding::replace_;
constexpr std::array<uint8_t, 16>  MinimalCanonicalEncoding::reverse_;

} // namespace sequence
} // namespace genesis