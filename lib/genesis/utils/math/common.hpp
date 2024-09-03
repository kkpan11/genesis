#ifndef GENESIS_UTILS_MATH_COMMON_H_
#define GENESIS_UTILS_MATH_COMMON_H_

/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2024 Lucas Czech

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
    Lucas Czech <lczech@carnegiescience.edu>
    Department of Plant Biology, Carnegie Institution For Science
    260 Panama Street, Stanford, CA 94305, USA
*/

/**
 * @brief
 *
 * @file
 * @ingroup utils
 */

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace genesis {
namespace utils {

// =================================================================================================
//     Constants and General Functions
// =================================================================================================

/**
 * @brief Make the world go round.
 */
constexpr double PI = 3.141592653589793238463;

inline double circumference( double radius )
{
    return 2.0 * PI * radius;
}

// =================================================================================================
//     Number Handling
// =================================================================================================

/**
 * @brief Sort two values in ascending order, inplace.
 */
template <typename T> inline
void ascending( T& f, T& s )
{
    if( f > s ) {
        std::swap( f, s );
    }
}

/**
 * @brief Sort two values in descending order, inplace.
 */
template <typename T> inline
void descending( T& f, T& s )
{
    if( f < s ) {
        std::swap( f, s );
    }
}

/**
 * @brief Calculate the absolute differenence between two values.
 *
 * This function is particularly useful for unsigned types, as subtracting them and then using
 * std::abs() does not work for them.
 */
template< typename T > inline constexpr
T abs_diff( T const& lhs, T const& rhs )
{
    return ((lhs > rhs) ? (lhs - rhs) : (rhs - lhs));
}

/**
 * @brief Implementation of signum(T x) for unsigned types. See there for details.
 */
template <typename T> inline constexpr
int signum(T x, std::false_type )
{
    // The tag type `std::false_type` is unnamed in order to avoid compiler warnings about an
    // unused parameter. As this function is `constexpr`, we cannot shut this warning down by
    // using `(void) param`, so making it unnamed is a reasonable solution in this case.
    return T(0) < x;
}

/**
 * @brief Implementation of signum(T x) for signed types. See there for details.
 */
template <typename T> inline constexpr
int signum(T x, std::true_type )
{
    // The tag type `std::false_type` is unnamed in order to avoid compiler warnings about an
    // unused parameter. As this function is `constexpr`, we cannot shut this warning down by
    // using `(void) param`, so making it unnamed is a reasonable solution in this case.
    return (T(0) < x) - (x < T(0));
}

/**
 * @brief Get the sign of a value, which is either -1, 0, or 1.
 *
 * Works for all numerical types. We use two tagged implementations for signed and unsigned types
 * in order to avoid compiler warnings. See
 * http://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
 * for details.
 */
template <typename T> inline constexpr
int signum(T x)
{
    return signum( x, std::is_signed<T>() );
}

/**
 * @brief Three-way comparison (spaceship operator) for C++ <20.
 *
 * Return `-1` if `lhs < rhs`, return `+1` if `lhs > rhs`, return `0` if `lhs == rhs`,
 * for arithmetic types.
 */
template <typename T, typename U>
inline constexpr int compare_threeway( T lhs, U rhs )
{
    // Branchless comparison of the values
    // https://stackoverflow.com/a/10997428/4184258
    static_assert( static_cast<int>( false ) == 0, "static_cast<int>( false ) != 0" );
    static_assert( static_cast<int>( true ) == 1,  "static_cast<int>( true ) != 1" );
    return static_cast<int>( lhs > rhs ) - static_cast<int>( lhs < rhs );
}

/**
 * @brief Check whether two doubles are almost equal, using a relative epsilon to compare them.
 */
inline bool almost_equal_relative(
    double lhs,
    double rhs,
    double max_rel_diff = std::numeric_limits<double>::epsilon()
) {
    // Calculate the difference.
    auto diff = std::abs( lhs - rhs );

    // Find the larger number.
    auto largest = std::max( std::abs( lhs ), std::abs( rhs ));

    // Do the comparison.
    return ( diff <= largest * max_rel_diff );
}

/**
 * @brief Retun the value of `x`, rounded to the decimal digit given by `accuracy_order`.
 */
inline double round_to( double x, size_t accuracy_order )
{
    double factor = std::pow( 10, accuracy_order );
    return std::round( x * factor ) / factor;
}

/**
 * @brief Calculate the power `base^exp` for positive integer values.
 *
 * Remark: This overflows quite easily. The function does not check whether the desired power
 * actually fits within `size_t`. Use is_valid_int_pow() to check for this first.
 *
 * Rationale for this function: One could argue that int powers are not really useful, particularly
 * because of the fast overflow. However, using doubles leads to rounding errors, at least for
 * bigger numbers. So, within the valid range, this function is more accurate. Also, it is faster.
 */
inline uint64_t int_pow( uint64_t base, uint8_t exp )
{
    // Using Exponentiation by squaring, see
    // http://stackoverflow.com/a/101613/4184258
    uint64_t result = 1;
    while( exp )  {
        if( exp & 1 ) {
            result *= base;
        }
        exp >>= 1;
        base *= base;
    }
    return result;
}

/**
 * @brief Return whether the given power can be stored within a `size_t`.
 *
 * Use int_pow() to calculate the actual value of the power.
 */
inline bool is_valid_int_pow( size_t base, size_t exp )
{
    return std::pow( base, exp ) < static_cast<double>( std::numeric_limits<size_t>::max() );
}

/**
 * @brief Square of a number.
 *
 * Simply \f$ x^2 \f$, but faster than `std::pow` for the low exponent. Meant as an abbreviation
 * where the argument is not already a variable, e.g., `squared( x - 1.0 )`,
 * without the need to store the intermediate argument term.
 */
inline constexpr double squared( double x )
{
    return x * x;
}

/**
 * @brief Cube of a number.
 *
 * Simply \f$ x^3 \f$, but faster than `std::pow` for the low exponent. Meant as an abbreviation
 * where the argument is not already a variable, e.g., `cubed( x - 1.0 )`,
 * without the need to store the intermediate argument term.
 */
inline constexpr double cubed( double x )
{
    return x * x * x;
}

// =================================================================================================
//     Helper Functions
// =================================================================================================

/**
 * @brief Helper function that cleans two ranges of `double` of the same length from non-finite values.
 *
 * This function is used for cleaning data input. It iterates both same-length ranges in parallel
 * and copies pairs elements to the two result vectors (one for each range), if both values are
 * finite. The result vectors thus have equal size.
 */
template <class ForwardIteratorA, class ForwardIteratorB>
std::pair<std::vector<double>, std::vector<double>> finite_pairs(
    ForwardIteratorA first_a, ForwardIteratorA last_a,
    ForwardIteratorB first_b, ForwardIteratorB last_b
) {
    // Prepare result.
    std::vector<double> vec_a;
    std::vector<double> vec_b;

    // Iterate in parallel.
    while( first_a != last_a && first_b != last_b ) {
        if( std::isfinite( *first_a ) && std::isfinite( *first_b ) ) {
            vec_a.push_back( *first_a );
            vec_b.push_back( *first_b );
        }
        ++first_a;
        ++first_b;
    }
    if( first_a != last_a || first_b != last_b ) {
        throw std::runtime_error(
            "Ranges need to have same length."
        );
    }

    assert( vec_a.size() == vec_b.size() );
    return { vec_a, vec_b };
}

/**
 * @brief Iterate two ranges of `double` values in parallel, and execute a function for
 * each pair of values from the two ranges where both values are finite.
 *
 * The ranges need to have the same length.
 */
template <class ForwardIteratorA, class ForwardIteratorB>
void for_each_finite_pair(
    ForwardIteratorA first_a, ForwardIteratorA last_a,
    ForwardIteratorB first_b, ForwardIteratorB last_b,
    std::function<void( double value_a, double value_b )> execute
) {
    // Iterate in parallel.
    while( first_a != last_a && first_b != last_b ) {
        if( std::isfinite( *first_a ) && std::isfinite( *first_b ) ) {
            execute( *first_a, *first_b );
        }
        ++first_a;
        ++first_b;
    }

    // Check if the ranges have the same length.
    if( first_a != last_a || first_b != last_b ) {
        throw std::runtime_error( "Ranges need to have same length." );
    }
}

} // namespace utils
} // namespace genesis

#endif // include guard
