/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2020 Lucas Czech

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
    Lucas Czech <lucas.czech@h-its.org>
    Exelixis Lab, Heidelberg Institute for Theoretical Studies
    Schloss-Wolfsbrunnenweg 35, D-69118 Heidelberg, Germany
*/

/**
 * @brief
 *
 * @file
 * @ingroup utils
 */

#include "genesis/utils/io/base64.hpp"

#include "genesis/utils/io/char.hpp"
#include "genesis/utils/core/logging.hpp"

#include <cassert>
#include <stdexcept>

namespace genesis {
namespace utils {

// =================================================================================================
//     Base 64 Encode/Decode
// =================================================================================================

// Code adaptem from https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64#C++

static const char base64_encode_lookup_[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64_pad_char_ = '=';

template<class T>
std::string base64_encode_( T const& input, size_t line_length )
{
    // Init and reserve space for result
    std::string encoded;
    size_t char_res = (( input.size() / 3 ) + ( input.size() % 3 > 0 )) * 4;
    if( line_length > 0 ) {
        char_res += char_res / line_length;
    }
    encoded.reserve( char_res );

    // We use a lambda to simplify putting chars, counting them, and wrapping lines as neccessary.
    size_t out_cnt = 0;
    auto put_char = [&]( char c ){
        // Put the char
        assert( encoded.size() + 1 <= encoded.capacity() );
        encoded.append( 1, c );

        // Line wrapping as needed
        ++out_cnt;
        if( line_length > 0 && out_cnt % line_length == 0 ) {
            encoded.append( 1, '\n' );
        }
    };

    // Process main part of the input
    std::uint32_t temp = 0;
    auto it = input.begin();
    for( std::size_t i = 0; i < input.size() / 3; ++i ) {
        // Convert to big endian
        temp  = ( *it++ ) << 16;
        temp += ( *it++ ) << 8;
        temp += ( *it++ );

        // Add to output
        put_char( base64_encode_lookup_[( temp & 0x00FC0000 ) >> 18 ]);
        put_char( base64_encode_lookup_[( temp & 0x0003F000 ) >> 12 ]);
        put_char( base64_encode_lookup_[( temp & 0x00000FC0 ) >> 6  ]);
        put_char( base64_encode_lookup_[( temp & 0x0000003F )       ]);
    }

    // Process remaining part and add padding
    switch( input.size() % 3 ) {
        case 2: {
            // Convert to big endian
            temp  = ( *it++ ) << 16;
            temp += ( *it++ ) << 8;

            // Add to output
            put_char( base64_encode_lookup_[( temp & 0x00FC0000 ) >> 18 ]);
            put_char( base64_encode_lookup_[( temp & 0x0003F000 ) >> 12 ]);
            put_char( base64_encode_lookup_[( temp & 0x00000FC0 ) >> 6  ]);
            put_char( base64_pad_char_);
            break;
        }
        case 1: {
            // Convert to big endian
            temp = (*it++) << 16;

            // Add to output
            put_char( base64_encode_lookup_[( temp & 0x00FC0000 ) >> 18 ]);
            put_char( base64_encode_lookup_[( temp & 0x0003F000 ) >> 12 ]);
            put_char( base64_pad_char_);
            put_char( base64_pad_char_);
            break;
        }
    }

    LOG_DBG << "input.size() " << input.size() << " char_res " << char_res << " encoded.size() " << encoded.size() << " encoded.capacity() " << encoded.capacity();

    // If our initial reservation was correct, we have reached exactly capacity.
    assert( encoded.size() == encoded.capacity() );
    return encoded;
}

template<class T>
T base64_decode_( std::string const& input )
{
    // Edge case.
    if( input.size() == 0 ) {
        return T{};
    }

    // Here, we are lazy (for now) and count the number of actual (non-new-line) chars directly.
    // This is fast enough for now. If this ever becomes a bottleneck, we shoudl refactor and
    // only loop once instead. On the other hand, that would prohibit properly reserving the
    // needed output space... Tradeoffs. But does not matter too much now. We just count first.
    size_t char_cnt = 0;
    for( auto c : input ) {
        // Use a non-branching counter to be as fast as possible.
        static_assert( (int)true == 1 && (int)false == 0, "Boolean counting does not work." );
        char_cnt += ! utils::is_space( c );
    }
    if( char_cnt % 4 ) {
        throw std::runtime_error( "Invalid base64 length that is not a multiple of 4");
    }

    // Get padding
    std::size_t padding = 0;
    assert( char_cnt >= 4 );
    if( input[ char_cnt - 1 ] == base64_pad_char_ ) {
        ++padding;
    }
    if( input[ char_cnt - 2 ] == base64_pad_char_ ) {
        ++padding;
    }

    // Init and reserve space for result
    T decoded;
    decoded.reserve( (( char_cnt / 4 ) * 3 ) - padding );

    // Hold decoded quanta
    std::uint32_t temp = 0;

    // We are expecting ASCII encoding here!
    static_assert( 0x41 == 'A' && 0x5A == 'Z', "Non-ASCII encoding detected. We expect ASCII." );
    static_assert( 0x61 == 'a' && 0x7A == 'z', "Non-ASCII encoding detected. We expect ASCII." );
    static_assert( 0x30 == '0' && 0x39 == '9', "Non-ASCII encoding detected. We expect ASCII." );
    static_assert( 0x2B == '+' && 0x2F == '/', "Non-ASCII encoding detected. We expect ASCII." );

    // Process the input
    auto it = input.begin();
    while( it < input.end() ) {

        // Each set of 4 (non-new-line) chars makes up 3 bytes of data.
        for( std::size_t i = 0; i < 4; ++i ) {

            // Skip new lines.
            while( utils::is_space( *it )) {
                ++it;
            }

            // Process the current char
            temp <<= 6;
            if ( *it >= 0x41 && *it <= 0x5A ) {
                temp |= *it - 0x41;
            } else if( *it >= 0x61 && *it <= 0x7A ) {
                temp |= *it - 0x47;
            } else if( *it >= 0x30 && *it <= 0x39 ) {
                temp |= *it + 0x04;
            } else if( *it == 0x2B ) {
                temp |= 0x3E;
            } else if( *it == 0x2F ) {
                temp |= 0x3F;
            } else if( *it == base64_pad_char_ ) {
                switch( input.end() - it ) {
                    case 1: {
                        //One pad character
                        assert( decoded.size() + 2 <= decoded.capacity() );
                        decoded.push_back(( temp >> 16 ) & 0x000000FF );
                        decoded.push_back(( temp >> 8  ) & 0x000000FF );
                        return decoded;
                    }
                    case 2: {
                        //Two pad characters
                        assert( decoded.size() + 1 <= decoded.capacity() );
                        decoded.push_back(( temp >> 10 ) & 0x000000FF );
                        return decoded;
                    }
                    default: {
                        throw std::runtime_error( "Invalid padding in base64 decoding" );
                    }
                }
            } else {
                throw std::runtime_error( "Invalid character in base64 decoding" );
            }

            ++it;
        }

        assert( decoded.size() + 3 <= decoded.capacity() );
        decoded.push_back(( temp >> 16 ) & 0x000000FF );
        decoded.push_back(( temp >> 8  ) & 0x000000FF );
        decoded.push_back(( temp       ) & 0x000000FF );
    }

    // If our initial reservation was correct, we have reached exactly capacity.
    assert( decoded.size() == decoded.capacity() );
    return decoded;
}

// =================================================================================================
//     Base 64 Container Conversion
// =================================================================================================

std::string base64_encode( std::vector<std::uint8_t> const& input, size_t line_length )
{
    return base64_encode_( input, line_length );
}

std::string base64_encode( std::string const& input, size_t line_length )
{
    return base64_encode_( input, line_length );
}

std::vector<std::uint8_t> base64_decode_uint8( std::string const& input )
{
    using ContainerType = std::vector<std::uint8_t>;
    return base64_decode_<ContainerType>( input );
}

std::string base64_decode_string( std::string const& input )
{
    using ContainerType = std::string;
    return base64_decode_<ContainerType>( input );
}

} // namespace utils
} // namespace genesis
