#ifndef GENESIS_UTILS_MATH_STATISTICS_H_
#define GENESIS_UTILS_MATH_STATISTICS_H_

/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2018 Lucas Czech and HITS gGmbH

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

#include "genesis/utils/core/algorithm.hpp"
#include "genesis/utils/math/ranking.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace genesis {
namespace utils {

// =================================================================================================
//     Structures and Classes
// =================================================================================================

/**
 * @brief Store a pair of min and max values.
 *
 * This notation is simply more readable than the std default of using a `pair<T, T>`.
 */
template< typename T >
struct MinMaxPair
{
    T min;
    T max;
};

/**
 * @brief Store a mean and a standard deviation value.
 *
 * This notation is simply more readable than the std default of using a `pair<T, T>` for such
 * types.
 */
struct MeanStddevPair
{
    double mean;
    double stddev;
};

/**
 * @brief Store the values of quartiles: `q0 == min`, `q1 == 25%`, `q2 == 50%`, `q3 == 75%`,
 * `q4 == max`.
 */
struct Quartiles
{
    double q0 = 0.0;
    double q1 = 0.0;
    double q2 = 0.0;
    double q3 = 0.0;
    double q4 = 0.0;
};

// =================================================================================================
//     Mean Stddev
// =================================================================================================

/**
 * @brief Calculate the arithmetic mean and standard deviation of a range of `double` elements.
 *
 * The iterators @p first and @p last need to point to a range of `double` values,
 * with @p last being the past-the-end element.
 * The function then calculates the arithmetic mean and standard deviation of all finite elements
 * in the range. If no elements are finite, or if the range is empty, both returned values are `0.0`.
 * Non-finite numbers are ignored.
 *
 * If the resulting standard deviation is below the given @p epsilon (e.g, `0.0000001`), it is
 * "corrected" to be `1.0` instead. This is an inelegant (but usual) way to handle near-zero values,
 * which for some use cases would cause problems like a division by zero later on.
 * By default, @p epsilon is `-1.0`, which deactivates this check - a standard deviation can never
 * be below `0.0`.
 *
 * @see mean_stddev( std::vector<double> const&, double epsilon ) for a version for `std::vector`.
 * @see arithmetic_mean() for a function that only calculates the mean, and thus saves the effort
 * of a second iteration over the range.
 */
template <class ForwardIterator>
MeanStddevPair mean_stddev( ForwardIterator first, ForwardIterator last, double epsilon = -1.0 )
{
    // Prepare result.
    MeanStddevPair result;
    result.mean   = 0.0;
    result.stddev = 0.0;
    size_t count  = 0;

    // Sum up elements.
    auto it = first;
    while( it != last ) {
        if( std::isfinite( *it ) ) {
            result.mean += *it;
            ++count;
        }
        ++it;
    }

    // If there are no valid elements, return an all-zero result.
    if( count == 0 ) {
        return result;
    }

    //  Calculate mean.
    result.mean /= static_cast<double>( count );

    // Calculate std dev.
    it = first;
    while( it != last ) {
        if( std::isfinite( *it ) ) {
            result.stddev += (( *it - result.mean ) * ( *it - result.mean ));
        }
        ++it;
    }
    assert( count > 0 );
    result.stddev /= static_cast<double>( count );
    result.stddev = std::sqrt( result.stddev );

    // The following in an inelegant (but usual) way to handle near-zero values,
    // which later would cause a division by zero.
    assert( result.stddev >= 0.0 );
    if( result.stddev <= epsilon ){
        result.stddev = 1.0;
    }

    return result;
}

/**
 * @brief Calculate the mean and standard deviation of a `std::vector` of `double` elements.
 *
 * @see mean_stddev( ForwardIterator first, ForwardIterator last, double epsilon ) for details.
 * @see arithmetic_mean() for a function that only calculates the mean, and thus saves the effort
 * of a second iteration over the range.
 */
inline MeanStddevPair mean_stddev( std::vector<double> const& vec, double epsilon = -1.0 )
{
    return mean_stddev( vec.begin(), vec.end(), epsilon );
}

/**
 * @brief Calculate the arithmetic mean of a range of numbers.
 *
 * The iterators @p first and @p last need to point to a range of `double` values,
 * with @p last being the past-the-end element.
 * The function then calculates the arithmetic mean of all finite elements in the range.
 * If no elements are finite, or if the range is empty, the returned value is `0.0`.
 * Non-finite numbers are ignored.
 *
 * @see arithmetic_mean( std::vector<double> const& ) for a version for `std::vector`.
 * @see mean_stddev() for a function that also calcualtes the standard deviation.
 * @see geometric_mean() for a function that calculates the geometric mean.
 */
template <class ForwardIterator>
double arithmetic_mean( ForwardIterator first, ForwardIterator last )
{
    // Prepare result.
    double mean  = 0.0;
    size_t count = 0;

    // Sum up elements.
    auto it = first;
    while( it != last ) {
        if( std::isfinite( *it ) ) {
            mean += *it;
            ++count;
        }
        ++it;
    }

    // If there are no valid elements, return an all-zero result.
    if( count == 0 ) {
        return mean;
    }

    //  Calculate mean.
    assert( count > 0 );
    return mean / static_cast<double>( count );
}

/**
 * @brief Calculate the arithmetic mean of a `std::vector` of `double` elements.
 *
 * @see arithmetic_mean( ForwardIterator first, ForwardIterator last ) for details.
 * @see mean_stddev() for a function that simultaneously calculates the standard deviation.
 * @see geometric_mean() for a function that calculates the geometric mean.
 */
inline double arithmetic_mean( std::vector<double> const& vec )
{
    return arithmetic_mean( vec.begin(), vec.end() );
}

/**
 * @brief Calculate the geometric mean of a range of positive numbers.
 *
 * The iterators @p first and @p last need to point to a range of `double` values,
 * with @p last being the past-the-end element.
 * The function then calculates the geometric mean of all positive finite elements in the range.
 * If no elements are finite, or if the range is empty, the returned value is `0.0`.
 * Non-finite numbers are ignored.
 * If finite non-positive numbers (zero or negative) are found, an exception is thrown.
 *
 * @see geometric_mean( std::vector<double> const& ) for a version for `std::vector`.
 * @see weighted_geometric_mean() for a weighted version.
 * @see arithmetic_mean() for a function that calculates the arithmetic mean.
 */
template <class ForwardIterator>
double geometric_mean( ForwardIterator first, ForwardIterator last )
{
    double prod  = 1.0;
    size_t count = 0;

    // Multiply elements.
    auto it = first;
    while( it != last ) {
        if( std::isfinite( *it ) ) {
            if( *it <= 0.0 ) {
                throw std::invalid_argument(
                    "Cannot calculate geometric mean of non-positive numbers."
                );
            }
            prod *= *it;
            ++count;
        }
        ++it;
    }

    // If there are no valid elements, return an all-zero result.
    if( count == 0 ) {
        return 0.0;
    }

    // Return the result.
    assert( prod  > 0.0 );
    assert( count > 0 );
    return std::pow( prod, 1.0 / static_cast<double>( count ));
}

/**
 * @brief Calculate the geometric mean of a `std::vector` of `double` elements.
 *
 * @see geometric_mean( ForwardIterator first, ForwardIterator last ) for details.
 * @see arithmetic_mean() for a function that calculates the arithmetic mean.
 */
inline double geometric_mean( std::vector<double> const& vec )
{
    return geometric_mean( vec.begin(), vec.end() );
}

/**
 * @brief Calculate the weighted geometric mean of a range of positive numbers.
 *
 * The iterators @p first_value and @p last_value, as well as @p first_weight and @p last_weight,
 * need to point to ranges of `double` values, with @p last_value and @p last_weight being the
 * past-the-end elements. Both ranges need to have the same size.
 * The function then calculates the weighted geometric mean of all positive finite elements
 * in the range. If no elements are finite, or if the range is empty, the returned value is `0.0`.
 * Non-finite numbers are ignored.
 * If finite non-positive numbers (zero or negative) are found, an exception is thrown.
 *
 * For a set of values \f$ v \f$ and a set of weights \f$ w \f$,
 * the weighted geometric mean \f$ g \f$ is calcualted following [1]:
 *
 * \f$ g = \exp \left( \frac{ \sum w \cdot \log v }{ \sum w } \right) \f$
 *
 * That is, if all weights are `1.0`, the formula yields the standard geometric mean.
 *
 * > [1] J. D. Silverman, A. D. Washburne, S. Mukherjee, and L. A. David,
 * > "A phylogenetic transform enhances analysis of compositional microbiota data,"
 * > Elife, vol. 6, p. e21887, Feb. 2017.
 * > https://elifesciences.org/articles/21887
 *
 * @see weighted_geometric_mean( std::vector<double> const& ) for a version for `std::vector`.
 * @see geometric_mean() for the unweighted version.
 * @see arithmetic_mean() for a function that calculates the arithmetic mean.
 */
template <class ForwardIterator>
double weighted_geometric_mean(
    ForwardIterator first_value,  ForwardIterator last_value,
    ForwardIterator first_weight, ForwardIterator last_weight
) {
    double num = 0.0;
    double den = 0.0;
    size_t cnt = 0;

    // Multiply elements.
    auto it_v = first_value;
    auto it_w = first_weight;
    while( it_v != last_value && it_w != last_weight ) {
        if( std::isfinite( *it_v ) && std::isfinite( *it_w )) {
            if( *it_v <= 0.0 ) {
                throw std::invalid_argument(
                    "Cannot calculate weighted geometric mean of non-positive values."
                );
            }
            if( *it_w < 0.0 ) {
                throw std::invalid_argument(
                    "Cannot calculate weighted geometric mean with negative weights."
                );
            }

            num += *it_w * std::log( *it_v );
            den += *it_w;
            ++cnt;
        }
        ++it_v;
        ++it_w;
    }

    // Range check
    if( it_v != last_value || it_w != last_weight ) {
        throw std::runtime_error(
            "The value and the weight ranges need to have same length "
            "to compute the weighted geometric mean."
        );
    }

    // If there are no valid elements, return an all-zero result.
    if( cnt == 0 ) {
        return 0.0;
    }

    // Return the result.
    assert( cnt > 0 );
    return std::exp( num / den );
}

/**
 * @brief Calculate the weighted geometric mean of a `std::vector` of `double` elements.
 *
 * @see weighted_geometric_mean( ForwardIterator first, ForwardIterator last ) for details.
 * @see geometric_mean() for the unweighted version.
 * @see arithmetic_mean() for a function that calculates the arithmetic mean.
 */
inline double weighted_geometric_mean(
    std::vector<double> const& values,
    std::vector<double> const& weights
) {
    return weighted_geometric_mean( values.begin(), values.end(), weights.begin(), weights.end() );
}

// =================================================================================================
//     Norms
// =================================================================================================

/**
 * @brief Calculate the p-norm of a range of numbers.
 *
 * The iterators @p first and @p last need to point to a range of `double` values,
 * with @p last being the past-the-end element. The parameter @p p has to be > 1.0.
 * In order to get the maximum norm (or infinity norm), @p p can also be set to positive infinity,
 * that is, \c std::numeric_limits<double>::infinity(). Default is `p == 2.0`, which is the Euclidean
 * norm.
 *
 * @see euclidean_norm(), manhattan_norm(), and maximum_norm() for special cases,
 * which simply call this function with a fixed @p p, in order to make code more expressive.
 * @see aitchison_norm() for another type of norm.
 */
template <class ForwardIterator>
double p_norm( ForwardIterator first, ForwardIterator last, double p = 2.0 )
{
    // Validity. We allow positive inifity.
    if( p < 1.0 || ( ! std::isfinite( p ) && ! std::isinf( p ))) {
        throw std::runtime_error( "Cannot calculate p-norm with p < 1.0" );
    }
    assert( p >= 1.0 );
    assert( std::isfinite( p ) || std::isinf( p ));

    double sum = 0.0;
    size_t cnt = 0;

    // Add vector elements.
    auto it = first;
    while( it != last ) {
        if( std::isfinite( *it ) ) {
            if( std::isfinite( p )) {
                sum += std::pow( std::abs( *it ), p );
            } else {
                sum = std::max( sum, *it );
            }
            ++cnt;
        }
        ++it;
    }

    // If there are no valid elements, return an all-zero result.
    if( cnt == 0 ) {
        return 0.0;
    }

    // Return the result.
    assert( cnt > 0 );
    if( std::isfinite( p )) {
        return std::pow( sum, 1.0 / p );
    } else {
        return sum;
    }

    // Make old compilers happy.
    return 0.0;
}

/**
 * @brief Calculate the p-norm of a `std::vector` of `double` elements.
 *
 * @see p_norm( ForwardIterator, ForwardIterator, double ) for details.
 * @see aitchison_norm() for another type of norm.
 */
inline double p_norm( std::vector<double> const& vec, double p = 2.0 )
{
    return p_norm( vec.begin(), vec.end(), p );
}

/**
 * @brief Calculate the Manhattan norm (L1 norm) of a range of numbers.
 *
 * The function is a more expressive version of p_norm( ForwardIterator, ForwardIterator, double )
 * with `p == 1.0`, in order to make code more expressive. See there for details.
 */
template <class ForwardIterator>
double manhattan_norm( ForwardIterator first, ForwardIterator last )
{
    return p_norm( first, last, 1.0 );
}

/**
 * @brief Calculate the Manhattan norm (L1 norm) of a `std::vector` of `double` elements.
 *
 * The function is a more expressive version of p_norm( std::vector<double> const&, double )
 * with `p == 1.0`, in order to make code more expressive. See there for details.
 */
inline double manhattan_norm( std::vector<double> const& vec )
{
    return p_norm( vec.begin(), vec.end(), 1.0 );
}

/**
 * @brief Calculate the Euclidean norm (L2 norm) of a range of numbers.
 *
 * The function is a more expressive version of p_norm( ForwardIterator, ForwardIterator, double )
 * with `p == 2.0`, in order to make code more expressive. See there for details.
 */
template <class ForwardIterator>
double euclidean_norm( ForwardIterator first, ForwardIterator last )
{
    return p_norm( first, last, 2.0 );
}

/**
 * @brief Calculate the Euclidean norm (L2 norm) of a `std::vector` of `double` elements.
 *
 * The function is a more expressive version of p_norm( std::vector<double> const&, double )
 * with `p == 2.0`, in order to make code more expressive. See there for details.
 */
inline double euclidean_norm( std::vector<double> const& vec )
{
    return p_norm( vec.begin(), vec.end(), 2.0 );
}

/**
 * @brief Calculate the Maximum norm (infinity norm) of a range of numbers.
 *
 * The function is a more expressive version of p_norm( ForwardIterator, ForwardIterator, double )
 * with \c p == std::numeric_limits<double>::infinity(), in order to make code more expressive.
 * See there for details.
 */
template <class ForwardIterator>
double maximum_norm( ForwardIterator first, ForwardIterator last )
{
    return p_norm( first, last, std::numeric_limits<double>::infinity() );
}

/**
 * @brief Calculate the Maximum norm (infinity norm) of a `std::vector` of `double` elements.
 *
 * The function is a more expressive version of p_norm( std::vector<double> const&, double )
 * with \c p == std::numeric_limits<double>::infinity(), in order to make code more expressive.
 * See there for details.
 */
inline double maximum_norm( std::vector<double> const& vec )
{
    return p_norm( vec.begin(), vec.end(), std::numeric_limits<double>::infinity() );
}

/**
 * @brief Calculate the Aitchison norm of a range of positive numbers.
 *
 * The iterators @p first and @p last need to point to a range of `double` values,
 * with @p last being the past-the-end element.
 *
 * Following [1], the Aitchison norm \f$ \| x \|_a \f$ of a vector \f$ x \f$ with \f$ s \f$ elements
 * is caluclated as
 *
 * \f$ \| x \|_a = \sqrt{ \frac{1}{2s} \sum_{j=1}^{s} \sum_{k=1}^{s} \left( \ln{ \frac{x_j}{x_k} } \right)^2 } \f$
 *
 * That is, the calculation is in \f$ \mathcal{O}( s^2 ) \f$.
 *
 * > [1] V. Pawlowsky-Glahn, J. J. Egozcue, and R. Tolosana-Delgado,
 * > "Modelling and Analysis of Compositional Data".
 * > Chichester, UK: John Wiley & Sons, Ltd, 2015.
 * > https://onlinelibrary.wiley.com/doi/book/10.1002/9781119003144
 *
 * @see p_norm(), euclidean_norm(), manhattan_norm(), and maximum_norm() for some standard norms.
 */
template <class ForwardIterator>
double aitchison_norm( ForwardIterator first, ForwardIterator last )
{
    double sum = 0.0;
    size_t cnt = 0;

    // Outer loop.
    auto it_out = first;
    while( it_out != last ) {
        if( std::isfinite( *it_out ) ) {

            if( *it_out <= 0.0 ) {
                throw std::invalid_argument(
                    "Cannot calculate Aitchison norm of non-positive values."
                );
            }

            // Inner loop.
            auto it_in = first;
            while( it_in != last ) {
                if( std::isfinite( *it_in ) ) {
                    auto const ln = std::log( *it_out / *it_in );
                    sum += ln * ln;
                }
                ++it_in;
            }

            ++cnt;
        }
        ++it_out;
    }

    // If there are no valid elements, return an all-zero result.
    if( cnt == 0 ) {
        return 0.0;
    }

    // Return the result.
    assert( cnt > 0 );
    return std::sqrt( sum / ( 2.0 * static_cast<double>( cnt )));
}

/**
 * @brief Calculate the Aitchison norm of a `std::vector` of `double` elements.
 *
 * @see aitchison_norm( ForwardIterator, ForwardIterator ) for details.
 */
inline double aitchison_norm( std::vector<double> const& vec )
{
    return aitchison_norm( vec.begin(), vec.end() );
}

// =================================================================================================
//     Median
// =================================================================================================

/**
 * @brief Calculate the median value of a sorted range of `double` values.
 *
 * The iterators are as usual: @p first points to the first element of the range,
 * @p last to the past-the-end element.
 *
 * The median of an odd sized range is its middle element; the median of an even sized range
 * is the arithmetic mean (average) of its two middle elements.
 */
template <class RandomAccessIterator>
double median( RandomAccessIterator first, RandomAccessIterator last )
{
    // Checks.
    if( ! std::is_sorted( first, last )) {
        throw std::runtime_error( "Range has to be sorted for median calculation." );
    }
    auto const size = static_cast<size_t>( std::distance( first, last ));
    if( size == 0 ) {
        return 0.0;
    }

    // Even or odd size? Median is calculated differently.
    if( size % 2 == 0 ) {

        // Get the two middle positions.
        size_t pl = size / 2 - 1;
        size_t pu = size / 2;
        assert( pl < size && pu < size );

        return ( *(first + pl) + *(first + pu) ) / 2.0;

    } else {

        // Int division, rounds down. This is what we want.
        size_t p = size / 2;
        assert( p < size );

        return *(first + p);
    }
}

/**
 * @brief Calculate the median value of a `vector` of `double`.
 *
 * The vector has to be sorted.
 */
inline double median( std::vector<double> const& vec )
{
    return median( vec.begin(), vec.end() );
}

// =================================================================================================
//     Quartiles
// =================================================================================================

/**
 * @brief Calculate the Quartiles of a sorted range of `double` values.
 *
 * The iterators are as usual: @p first points to the first element of the range,
 * @p last to the past-the-end element.
 */
template <class RandomAccessIterator>
Quartiles quartiles( RandomAccessIterator first, RandomAccessIterator last )
{
    // Prepare result.
    Quartiles result;

    // Checks.
    if( ! std::is_sorted( first, last )) {
        throw std::runtime_error( "Range has to be sorted for quartiles calculation." );
    }
    auto const size = static_cast<size_t>( std::distance( first, last ));
    if( size == 0 ) {
        return result;
    }

    // Set min, 50% and max.
    result.q0 = *first;
    result.q2 = median( first, last );
    result.q4 = *(first + size - 1);

    // Even or odd size? Quartiles are calculated differently.
    // This could be done shorter, but this way is more expressive.
    if( size % 2 == 0 ) {

        // Even: Split exaclty in halves.
        result.q1 = median( first, first + size / 2 );
        result.q3 = median( first + size / 2, first + size );

    } else {

        // Odd: Do not include the median value itself.
        result.q1 = median( first, first + size / 2 );
        result.q3 = median( first + size / 2 + 1, first + size );
    }

    return result;
}

/**
 * @brief Calculate the Quartiles of a `vector` of `double`.
 *
 * The vector has to be sorted.
 */
inline Quartiles quartiles( std::vector<double> const& vec )
{
    return quartiles( vec.begin(), vec.end() );
}

// =================================================================================================
//     Dispersion
// =================================================================================================

/**
 * @brief Calculate the index of dispersion.
 *
 * The coefficient of variation (CV), also known as the relative standard deviation (RSD),
 * is defined as the ratio of the standard deviation to the mean.
 * See mean_stddev() to calcualte those values.
 * See https://en.wikipedia.org/wiki/Coefficient_of_variation for details.
 */
inline double coefficient_of_variation( MeanStddevPair const& ms )
{
    return ms.stddev / ms.mean;
}

/**
 * @copydoc coefficient_of_variation( MeanStddevPair const& ms )
 */
inline std::vector<double> coefficient_of_variation( std::vector<MeanStddevPair> const& ms )
{
    auto res = std::vector<double>( ms.size() );
    for( size_t i = 0; i < ms.size(); ++i ) {
        res[ i ] = coefficient_of_variation( ms[i] );
    }
    return res;
}

/**
 * @brief Calculate the index of dispersion.
 *
 * The index of dispersion, also known as the dispersion index, coefficient of dispersion,
 * relative variance, variance-to-mean ratio (VMR) or Fano factor, is defined as the ratio of the
 * variance to the mean. Variance is the square of the standard deviation.
 * See mean_stddev() to calcualte those values.
 * See https://en.wikipedia.org/wiki/Index_of_dispersion for details.
 */
inline double index_of_dispersion( MeanStddevPair const& ms )
{
    return ms.stddev * ms.stddev / ms.mean;
}

/**
 * @copydoc index_of_dispersion( MeanStddevPair const& ms )
 */
inline std::vector<double> index_of_dispersion( std::vector<MeanStddevPair> const& ms )
{
    auto res = std::vector<double>( ms.size() );
    for( size_t i = 0; i < ms.size(); ++i ) {
        res[ i ] = index_of_dispersion( ms[i] );
    }
    return res;
}

/**
 * @brief Calculate the quartile_coefficient_of_dispersion.
 *
 * The quartile coefficient of dispersion is defined as `( Q3 - Q1 ) / ( Q3 + Q1 )`.
 * See quartiles() to caculate those values.
 * See https://en.wikipedia.org/wiki/Quartile_coefficient_of_dispersion for details.
 */
inline double quartile_coefficient_of_dispersion( Quartiles const& q )
{
    return ( q.q3 - q.q1 ) / ( q.q3 + q.q1 );
}

/**
 * @copydoc quartile_coefficient_of_dispersion( Quartiles const& ms )
 */
inline std::vector<double> quartile_coefficient_of_dispersion( std::vector<Quartiles> const& q )
{
    auto res = std::vector<double>( q.size() );
    for( size_t i = 0; i < q.size(); ++i ) {
        res[ i ] = quartile_coefficient_of_dispersion( q[i] );
    }
    return res;
}

// =================================================================================================
//     Correlation Coefficients
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
 * @brief Calculate the Pearson Correlation Coefficient between two ranges of `double`.
 *
 * Both ranges need to have the same length. Then, the function calculates the PCC
 * between the pairs of entries of both ranges. It skipes entries where any of the two values
 * is not finite.
 *
 * If each pair of entries in the ranges contains at leat one non-finite value, that is, if there
 * are no pairs of finite values, a `quiet_NaN` is returned. Furtheremore, if one of the ranges
 * has a standard deviation of `0.0`, e.g., because all its entries are `0.0` themselves,
 * a division by 0 occurs, leading to a `NaN` as well.
 */
template <class ForwardIteratorA, class ForwardIteratorB>
double pearson_correlation_coefficient(
    ForwardIteratorA first_a, ForwardIteratorA last_a,
    ForwardIteratorB first_b, ForwardIteratorB last_b
) {
    // Calculate means.
    double mean_a = 0.0;
    double mean_b = 0.0;
    size_t count = 0;
    auto it_a = first_a;
    auto it_b = first_b;
    while( it_a != last_a && it_b != last_b ) {
        if( std::isfinite( *it_a ) && std::isfinite( *it_b ) ) {
            mean_a += *it_a;
            mean_b += *it_b;
            ++count;
        }
        ++it_a;
        ++it_b;
    }
    if( it_a != last_a || it_b != last_b ) {
        throw std::runtime_error(
            "Ranges need to have same length to calculate their Pearson Correlation Coefficient."
        );
    }
    if( count == 0 ) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    assert( count > 0 );
    mean_a /= static_cast<double>( count );
    mean_b /= static_cast<double>( count );

    // Calculate PCC parts.
    double numerator = 0.0;
    double stddev_a  = 0.0;
    double stddev_b  = 0.0;
    it_a = first_a;
    it_b = first_b;
    while( it_a != last_a && it_b != last_b ) {
        if( std::isfinite( *it_a ) && std::isfinite( *it_b ) ) {
            double const d1 = *it_a - mean_a;
            double const d2 = *it_b - mean_b;
            numerator += d1 * d2;
            stddev_a  += d1 * d1;
            stddev_b  += d2 * d2;
        }
        ++it_a;
        ++it_b;
    }
    assert( it_a == last_a && it_b == last_b );

    // Calcualte PCC, and assert that it is in the correct range
    // (or not a number, which can happen if the std dev is 0.0, e.g. in all-zero vectors).
    auto const pcc = numerator / ( std::sqrt( stddev_a ) * std::sqrt( stddev_b ) );
    assert(( -1.0 <= pcc && pcc <= 1.0 ) || ( ! std::isfinite( pcc ) ));
    return pcc;
}

/**
 * @brief Calculate the Pearson Correlation Coefficient between the entries of two vectors.
 *
 * @copydetails pearson_correlation_coefficient( ForwardIteratorA first_a, ForwardIteratorA last_a, ForwardIteratorB first_b, ForwardIteratorB last_b ).
 */
inline double pearson_correlation_coefficient(
    std::vector<double> const& vec_a,
    std::vector<double> const& vec_b
) {
    return pearson_correlation_coefficient(
        vec_a.begin(), vec_a.end(), vec_b.begin(), vec_b.end()
    );
}

/**
 * @brief Calculate Spearman's Rank Correlation Coefficient between two ranges of `double`.
 *
 * Both ranges need to have the same length. Then, the function calculates Spearmans's Rho
 * between the pairs of entries of both vectors. Ranking is done via
 * @link ranking_fractional() fractional ranking@endlink.
 * Pairs of entries which contain non-finite values are skipped.
 */
template <class RandomAccessIteratorA, class RandomAccessIteratorB>
double spearmans_rank_correlation_coefficient(
    RandomAccessIteratorA first_a, RandomAccessIteratorA last_a,
    RandomAccessIteratorB first_b, RandomAccessIteratorB last_b
) {
    // Get cleaned results.
    auto const cleaned = finite_pairs( first_a, last_a, first_b, last_b );

    // Get the ranking of both vectors.
    auto ranks_a = ranking_fractional( cleaned.first );
    auto ranks_b = ranking_fractional( cleaned.second );
    assert( ranks_a.size() == ranks_b.size() );

    // Nice try. But removing them later does not work, as the ranges would be calculated
    // differently... So we have to live with making copies of the data :-(
    //
    // if( ranks_a.size() != ranks_b.size() ) {
    //     throw std::runtime_error(
    //         "Ranges need to have same length to calculate their "
    //         "Spearman's Rank Correlation Coefficient."
    //     );
    // }
    //
    // // Remove non finite values.
    // size_t ins = 0;
    // size_t pos = 0;
    // while( first_a != last_a && first_b != last_b ) {
    //     if( std::isfinite( *first_a ) && std::isfinite( *first_b ) ) {
    //         ranks_a[ ins ] = ranks_a[ pos ];
    //         ranks_b[ ins ] = ranks_b[ pos ];
    //         ++ins;
    //     }
    //     ++first_a;
    //     ++first_b;
    //     ++pos;
    // }
    //
    // // We already checked that the ranks have the same length. Assert this.
    // assert( first_a == last_a && first_b == last_b );
    // assert( pos == ranks_a.size() && pos == ranks_b.size() );
    //
    // // Erase the removed elements.
    // assert( ins <= ranks_a.size() && ins <= ranks_b.size() );
    // ranks_a.resize( ins );
    // ranks_b.resize( ins );

    return pearson_correlation_coefficient( ranks_a, ranks_b );
}

/**
 * @brief Calculate Spearman's Rank Correlation Coefficient between the entries of two vectors.
 *
 * @copydetails spearmans_rank_correlation_coefficient( RandomAccessIteratorA first_a, RandomAccessIteratorA last_a, RandomAccessIteratorB first_b, RandomAccessIteratorB last_b )
 */
inline double spearmans_rank_correlation_coefficient(
    std::vector<double> const& vec_a,
    std::vector<double> const& vec_b
) {
    return spearmans_rank_correlation_coefficient(
        vec_a.begin(), vec_a.end(), vec_b.begin(), vec_b.end()
    );
}

/**
 * @brief Apply Fisher z-transformation to a correlation coefficient.
 *
 * The coefficient can be calculated with pearson_correlation_coefficient() or
 * spearmans_rank_correlation_coefficient() and has to be in range `[ -1.0, 1.0 ]`.
 *
 * There is also a version of this function for a vector of coefficients.
 * See also matrix_col_pearson_correlation_coefficient(),
 * matrix_row_pearson_correlation_coefficient(), matrix_col_spearmans_rank_correlation_coefficient()
 * and matrix_row_spearmans_rank_correlation_coefficient() for matrix versions.
 */
inline double fisher_transformation( double correlation_coefficient )
{
    auto const r = correlation_coefficient;
    if( r < -1.0 || r > 1.0 ) {
        throw std::invalid_argument(
            "Cannot apply fisher transformation to value " + std::to_string( r ) +
            " outside of [ -1.0, 1.0 ]."
        );
    }

    // LOG_DBG << "formula " << 0.5 * log( ( 1.0 + r ) / ( 1.0 - r ) );
    // LOG_DBG << "simple  " << std::atanh( r );
    return std::atanh( r );
}

/**
 * @brief Apply Fisher z-transformation to a vector of correlation coefficients.
 *
 * See fisher_transformation( double ) for details.
 */
inline std::vector<double> fisher_transformation( std::vector<double> const& correlation_coefficients )
{
    auto res = correlation_coefficients;
    for( auto& elem : res ) {
        elem = fisher_transformation( elem );
    }
    return res;
}

} // namespace utils
} // namespace genesis

#endif // include guard
