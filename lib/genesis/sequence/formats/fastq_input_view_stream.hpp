#ifndef GENESIS_SEQUENCE_FORMATS_FASTQ_INPUT_VIEW_STREAM_H_
#define GENESIS_SEQUENCE_FORMATS_FASTQ_INPUT_VIEW_STREAM_H_

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

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)

#include "genesis/sequence/sequence.hpp"
#include "genesis/utils/core/std.hpp"
#include "genesis/utils/io/input_source.hpp"
#include "genesis/utils/io/input_stream.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace genesis {
namespace sequence {

// =================================================================================================
//     Fasta and Fastq Input Stream
// =================================================================================================

/**
 * @brief Stream through an input source and parse it as Fastq sequences, returning string views.
 *
 * This class allows to iterate over an input source, interpreting it as Fastq sequences,
 * and yielding one such sequence per iteration step, as simple string views into the four
 * components of a fastq record. This is useful for fast processing large files
 * without having to keep them fully in memory, or even allocate strings.
 *
 * In order to allow for the speed, this iterator does not make copies of the data. The returned
 * views are invalidated when incrementing the iterator. Furthermore, the input fastq file needs
 * to be of a stricter format than what the FastqInputStream can handle:
 *
 *   * Each record needs to consist of exactly four lines: Label, sequence, label again, quality.
 *     No line breaks are allowed within the sequence or quality strings.
 *   * The total length of a record cannot exceed the internal buffer length of the input stream,
 *     which at the time of writing is set to 4MB. Assuming short labels, that means that the
 *     sequence length cannot be more than ~2MB, plus ~2MB for the quality length.
 *
 * This stream is hence meant for short reads. It barely does any error checking, in order to allow
 * for maximum speed. We hence assume correct input files, and might crash unexpectedly if
 * malformed data is used in downstream processing.
 *
 * Example:
 *
 *     for( auto const& s : FastqInputViewStream( from_file( "/path/to/large_file.fastq" ))) {
 *         std::cout << s.sites() << "\n";
 *     }
 *
 * Use functions such as utils::from_file() and utils::from_string() to conveniently
 * get an input source that can be used here.
 *
 * Thread safety: No thread safety. The common use case for this iterator is to loop over a file.
 * Thus, guarding induces unnecessary overhead. If multiple threads read from this iterator, both
 * dereferencing and incrementing need to be guarded.
 */
class FastqInputViewStream
{
public:

    // -------------------------------------------------------------------------
    //     Member Types
    // -------------------------------------------------------------------------

    using self_type         = FastqInputViewStream;
    using value_type        = Sequence;
    using pointer           = value_type*;
    using reference         = value_type&;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    // ======================================================================================
    //      Internal Iterator
    // ======================================================================================

public:

    /**
     * @brief Internal iterator over the sequences.
     *
     * This is the class that does the actual work. Use the dereference `operator*()`
     * and `operator->()` to get the current Sequence of the iteration.
     */
    class Iterator
    {
        // -------------------------------------------------------------------------
        //     Typedefs and Enums
        // -------------------------------------------------------------------------

    public:

        using self_type         = FastqInputViewStream::Iterator;
        using value_type        = std::array<std::string_view, 4>;
        using pointer           = value_type const*;
        using reference         = value_type const&;
        using difference_type   = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        // -------------------------------------------------------------------------
        //     Constructors and Rule of Five
        // -------------------------------------------------------------------------

    private:

        Iterator() = default;

        Iterator( FastqInputViewStream const* parent )
            : parent_( parent )
        {
            // Safeguard
            if( ! parent_ ) {
                return;
            }

            // Start reading from the input source into a stream.
            input_stream_ = std::make_shared<utils::InputStream>( parent_->input_source_ );

            // Start streaming the data
            increment_();
        }

    public:

        ~Iterator() = default;

        Iterator( self_type const& ) = default;
        Iterator( self_type&& )      = default;

        Iterator& operator= ( self_type const& ) = default;
        Iterator& operator= ( self_type&& )      = default;

        friend FastqInputViewStream;

        // -------------------------------------------------------------------------
        //     Iterator Accessors
        // -------------------------------------------------------------------------

        self_type const* operator->() const
        {
            return this;
        }

        self_type* operator->()
        {
            return this;
        }

        self_type const& operator*() const
        {
            return *this;
        }

        self_type& operator*()
        {
            return *this;
        }

        // -------------------------------------------------------------------------
        //     Iteration
        // -------------------------------------------------------------------------

        self_type& operator ++ ()
        {
            increment_();
            return *this;
        }

        /**
         * @brief Compare two iterators for equality.
         *
         * Any two iterators that are created by calling begin() on the same
         * FastqInputViewStream instance will compare equal, as long as neither of them is
         * past-the-end. A valid (not past-the-end) iterator and an end() iterator will not compare
         * equal; all past-the-end iterators compare equal, independently from which parent they
         * were created.
         */
        bool operator==( self_type const& it ) const
        {
            return parent_ == it.parent_;
        }

        bool operator!=( self_type const& it ) const
        {
            return !(*this == it);
        }

        // -------------------------------------------------------------------------
        //     Sequence Access
        // -------------------------------------------------------------------------

        /**
         * @brief Get the label of the sequence.
         *
         * This is the firse line of the sequence, with no checks performed about the character set.
         */
        std::string_view const& label() const
        {
            return sequence_view_[0];
        }

        /**
         * @brief Get the first label line.
         *
         * This is an alias for label(), and provided as a means of distinction with label2().
         */
        std::string_view const& label1() const
        {
            return sequence_view_[0];
        }

        /**
         * @brief Get the sequence sites.
         *
         * This contains the sequence sites as they are in the input. No checks on their character
         * set or site casing are performed.
         */
        std::string_view const& sites() const
        {
            return sequence_view_[1];
        }

        /**
         * @brief Get the second label line.
         *
         * This is usally either empty or identical to the first label line. We do not check this,
         * and just return the data as it was in the input.
         */
        std::string_view const& label2() const
        {
            return sequence_view_[2];
        }

        /**
         * @brief Get the quality string.
         *
         * This contains just the characters as they are in the input. In order to decode them
         * into more usable phred scores or similar, use functions such as
         * quality_decode_to_phred_score() on the returned string.
         */
        std::string_view const& quality() const
        {
            return sequence_view_[3];
        }

        // -------------------------------------------------------------------------
        //     Internal Members
        // -------------------------------------------------------------------------

    private:

        // ---------------------------------------------
        //     Increment and Processing Samples
        // ---------------------------------------------

        void increment_()
        {
            assert( parent_ );

            // Check whether the input stream is good (not end-of-stream) and can be read from.
            // If not, we reached its end, so we stop reading in the next iteration.
            if( ! input_stream_ || ! *input_stream_ ) {
                parent_ = nullptr;
                input_stream_ = nullptr;
                sequence_view_ = std::array<std::string_view, 4>();
                return;
            }

            // Get the next record. Also give a more user friendly error if this does not work.
            try {
                sequence_view_ = input_stream_->get_line_views<4>();
            } catch( std::exception const& ex ) {
                throw std::runtime_error(
                    "Cannot stream through fastq " + input_stream_->source_name() +
                    " with fast string view parser, either because the file is corrupt, "
                    "or has lines that are too long. Error: " + ex.what()
                );
            }

            // Parse label 1
            if( sequence_view_[0].size() < 1 || sequence_view_[0][0] != '@' ) {
                throw std::runtime_error(
                    "Malformed fastq " + input_stream_->source_name() + ": Expecting '@' at "
                    "beginning of label near line " + std::to_string( input_stream_->line() ) +
                    ". Note that we here can only process fastq with single lines for the " +
                    "sequence and quality data."
                );
            }
            sequence_view_[0].remove_prefix( 1 );

            // Parse label 2
            if( sequence_view_[2].size() < 1 || sequence_view_[2][0] != '+' ) {
                throw std::runtime_error(
                    "Malformed fastq " + input_stream_->source_name() + ": Expecting '+' at "
                    "beginning of label near line " + std::to_string( input_stream_->line() ) +
                    ". Note that we here can only process fastq with single lines for the " +
                    "sequence and quality data."
                );
            }
            sequence_view_[2].remove_prefix( 1 );

            // Basic check of sequence and quality length.
            if( sequence_view_[1].empty() ) {
                throw std::runtime_error(
                    "Malformed fastq " + input_stream_->source_name() + ": Expecting a " +
                    "sequence sites line after the first label line near line "
                    + std::to_string( input_stream_->line() ) +
                    ". Note that we here can only process fastq with single lines for the " +
                    "sequence and quality data."
                );
            }
            if( sequence_view_[1].size() != sequence_view_[3].size() ) {
                throw std::runtime_error(
                    "Malformed fastq " + input_stream_->source_name() + ": Expecting the " +
                    "quality scores to be of the same length as the sequence near line " +
                    std::to_string( input_stream_->line() ) +
                    ". Note that we here can only process fastq with single lines for the " +
                    "sequence and quality data."
                );
            }
        }

        // -------------------------------------------------------------------------
        //     Data Members
        // -------------------------------------------------------------------------

    private:

        // Parent. If null, this indicates the end of the input and that we are done iterating.
        FastqInputViewStream const* parent_ = nullptr;

        // Data stream to read from.
        std::shared_ptr<utils::InputStream> input_stream_;

        // The sequence that we parse the input into and expose to the user.
        std::array<std::string_view, 4> sequence_view_;
    };

    // ======================================================================================
    //      Main Class
    // ======================================================================================

    // -------------------------------------------------------------------------
    //     Constructors and Rule of Five
    // -------------------------------------------------------------------------

    /**
     * @brief Create a default instance, with no input.
     */
    FastqInputViewStream()
        : input_source_( nullptr )
    {}

    /**
     * @brief Create an instance that reads from an input source.
     */
    explicit FastqInputViewStream(
        std::shared_ptr<utils::BaseInputSource> source
    )
        : input_source_( source )
    {}

    ~FastqInputViewStream() = default;

    FastqInputViewStream( self_type const& ) = default;
    FastqInputViewStream( self_type&& )      = default;

    self_type& operator= ( self_type const& ) = default;
    self_type& operator= ( self_type&& )      = default;

    // -------------------------------------------------------------------------
    //     Iteration
    // -------------------------------------------------------------------------

    Iterator begin() const
    {
        return Iterator( this );
    }

    Iterator end() const
    {
        return Iterator();
    }

    // -------------------------------------------------------------------------
    //     Settings
    // -------------------------------------------------------------------------

    std::shared_ptr<utils::BaseInputSource> input_source() const
    {
        return input_source_;
    }

    // -------------------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------------------

private:

    std::shared_ptr<utils::BaseInputSource> input_source_;
};

} // namespace sequence
} // namespace genesis

#endif // ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#endif // include guard
