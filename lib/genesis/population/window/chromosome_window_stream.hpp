#ifndef GENESIS_POPULATION_WINDOW_CHROMOSOME_WINDOW_STREAM_H_
#define GENESIS_POPULATION_WINDOW_CHROMOSOME_WINDOW_STREAM_H_

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
 * @ingroup population
 */

#include "genesis/population/window/base_window_stream.hpp"
#include "genesis/population/window/window_view.hpp"
#include "genesis/sequence/sequence_dict.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace genesis {
namespace population {

// =================================================================================================
//     Chromosome Window Stream
// =================================================================================================

/**
 * @brief Stream for traversing each chromosome as a whole, with an inner WindowView iterator
 * over the positions of each chromosome.
 *
 * With each step of the iteration, an inner WindowView iterator is yielded that traverses all
 * positions on a chromosome of the underlying input data stream.
 * Then, when incrementing the main iterator, we move forward to the next chromosome.
 *
 * This class is merely meant as a simplification over manually keeping track of the current
 * chromosome, for example when computing a statistic for whole chromosomes, so that those
 * algorithms do not need to take care of when to produce their output.
 * Instead, they can simply use this class, and produce output at each step of the main iterator.
 * This class contains a quite unfortunate amount of boiler plate, but hopefully makes downstream
 * algorithms easier to write.
 *
 * The three functors
 *
 *  * #entry_input_function,
 *  * #chromosome_function, and
 *  * #position_function
 *
 * have to be set in the class prior to starting the iteration for the chromosome iterator.
 * See make_chromosome_window_stream() and make_default_chromosome_window_stream()
 * for helper functions that take care of this for most of our data types.
 *
 * See BaseWindowStream for more details on the three functors, the template parameters.
 * This class here however does not derive from the BaseWindowStream over normal Window%s,
 * but behaves in a similar way - with the exception that it does not produce Window%s in each
 * step of the iteration, as we do not want to keep the positions of a whole chromosome in memory.
 * Hence, instead, it yields a WindowView iterator, directly streaming over the positions of the
 * chromosome, without keeping all data in memory.
 *
 * @see make_chromosome_window_stream()
 * @see make_default_chromosome_window_stream()
 */
template<class InputStreamIterator, class DataType = typename InputStreamIterator::value_type>
class ChromosomeWindowStream final : public BaseWindowStream<
    InputStreamIterator, DataType, ::genesis::population::WindowView<DataType>
>
{
public:

    // -------------------------------------------------------------------------
    //     Typedefs and Enums
    // -------------------------------------------------------------------------

    using WindowViewType = ::genesis::population::WindowView<DataType>;
    using self_type = ChromosomeWindowStream<InputStreamIterator, DataType>;
    using base_type = BaseWindowStream<InputStreamIterator, DataType, WindowViewType>;

    // The input types that we take from the underlying stream over genome positions.
    using InputType         = typename InputStreamIterator::value_type;
    // using Entry             = typename Window::Entry;

    // This class produces an iterator of type WindowView.
    // That WindowView then iterates over the actual values of the input.
    using iterator_category = std::input_iterator_tag;
    using value_type        = WindowViewType;
    using pointer           = value_type*;
    using reference         = value_type&;
    using const_reference   = value_type const&;

    // ======================================================================================
    //      Internal Iterator
    // ======================================================================================

    /**
     * @brief Internal iterator that produces WindowView%s.
     */
    class DerivedIterator final : public BaseWindowStream<
        InputStreamIterator, DataType, WindowViewType
    >::BaseIterator
    {
    public:

        // -------------------------------------------------------------------------
        //     Constructors and Rule of Five
        // -------------------------------------------------------------------------

        using self_type = typename ChromosomeWindowStream<
            InputStreamIterator, DataType
        >::DerivedIterator;

        // using base_iterator_type = typename base_type::BaseIterator;
        using base_iterator_type = typename BaseWindowStream<
            InputStreamIterator, DataType, WindowViewType
        >::BaseIterator;

        // using WindowViewType    = WindowViewType;
        // using Window            = ::genesis::population::Window<DataType>;
        // using Entry             = typename Window::Entry;
        using InputType         = typename InputStreamIterator::value_type;

        using iterator_category = std::input_iterator_tag;
        using value_type        = WindowViewType;
        using pointer           = value_type*;
        using reference         = value_type&;
        using const_reference   = value_type const&;

    private:

        DerivedIterator() = default;

        DerivedIterator(
            ChromosomeWindowStream const* parent
        )
            : base_iterator_type( parent )
            , parent_( parent )
            // , window_( base_iterator_type::current_, base_iterator_type::end_ )
            // , window_( parent )
        {
            // Edge case check. See Base for details.
            if( ! parent_ ) {
                return;
            }

            // For this particular iterator, where we process the whole chromosome or genome,
            // we are always at the "first" and "last" window of a chromosome, in a sense...
            base_iterator_type::is_first_window_ = true;
            base_iterator_type::is_last_window_ = true;

            // Let's get going.
            increment_();
        }

    public:

        virtual ~DerivedIterator() override = default;

        DerivedIterator( self_type const& ) = default;
        DerivedIterator( self_type&& )      = default;

        DerivedIterator& operator= ( self_type const& ) = default;
        DerivedIterator& operator= ( self_type&& )      = default;

        friend ChromosomeWindowStream;

        // -------------------------------------------------------------------------
        //     Internal and Virtual Members
        // -------------------------------------------------------------------------

    private:

        void increment_() override final
        {
            // Check that we are still good. If not, this function being called is likely a user
            // error by trying to increment a past-the-end iterator.
            assert( parent_ );

            // Move to the next chromosome. This is only important if this increment function
            // is called before the inner window view iterator has finished the whole chromosome,
            // so if for example a break is called within.
            while(
                base_iterator_type::current_ != base_iterator_type::end_ &&
                parent_->chromosome_function( *base_iterator_type::current_ ) == window_.chromosome()
            ) {
                ++base_iterator_type::current_;
            }

            // Now check whether there is any data left. If not, we are done here.
            if( base_iterator_type::current_ == base_iterator_type::end_ ) {
                parent_ = nullptr;
                return;
            }
            assert( parent_ );

            // Now we know there is still data, but it belongs to a different chromosome.
            assert( base_iterator_type::current_ != base_iterator_type::end_ );
            assert(
                parent_->chromosome_function( *base_iterator_type::current_ ) != window_.chromosome()
            );

            // We need pointer variables to the iterators and other elements,
            // which can be used as copy-inits for the lambda below.
            // We need to access the underlying iterator through the self type of the class,
            // see here https://stackoverflow.com/a/28623297/4184258
            // (we could do this in all other instances as well, but it only matters here).
            bool is_first = true;
            auto& cur = self_type::current_;
            auto& end = self_type::end_;
            auto const par = parent_;
            auto const chr = parent_->chromosome_function( *base_iterator_type::current_ );
            auto const seq_dict = parent_->sequence_dict_;

            // Check that we do not have invalid data where chromosomes are repeated.
            if( processed_chromosomes_.count( chr ) > 0 ) {
                throw std::runtime_error(
                    "Chromosome " + chr + " occurs multiple times in the input."
                );
            }
            processed_chromosomes_.insert( chr );

            // We reset the window view, so that it's a new iterator for the new chromosome.
            window_ = WindowViewType();
            window_.chromosome( chr );
            if( parent_->sequence_dict_ ) {
                auto const dict_entry = parent_->sequence_dict_->find( chr );
                if( dict_entry == parent_->sequence_dict_->end() ) {
                    throw std::invalid_argument(
                        "In ChromosomeWindowStream: Cannot iterate chromosome \"" + chr +
                        "\", as the provided sequence dictionary or reference genome "
                        "does not contain the chromosome."
                    );
                }
                window_.first_position( 1 );
                window_.last_position( dict_entry->length );
            } else {
                window_.first_position( 1 );
                window_.last_position(  1 );
            }
            auto& window = window_;

            // Iterate starting from the first position, with a fitting increment function.
            window_.get_element = [
                is_first, &cur, &end, par, chr, seq_dict, &window
            ]() mutable -> DataType* {
                // If this is the first call of the function, we are initializing the WindowView
                // with the current entry of the underlying iterator. If not, we first move to the
                // next position (if there is any), before getting the data.
                if( is_first ) {
                    assert( cur != end );
                    is_first = false;
                    return &*cur;
                }

                // Now we are in the case that we want to move to the next position first.
                // Move to the next position.
                assert( cur != end );
                auto const old_pos = par->position_function( *cur );
                ++cur;

                // Check whether we are done with the chromosome.
                // If not, we update the last position to be the one that we just found,
                // and return the current element that we just moved to.
                if( cur == end || par->chromosome_function( *cur ) != chr ) {

                    // If we reach the end of a chromosome, we check that its length is within
                    // the dict limits, just as a safety measure. Only check once, to avoid
                    // looking up the chromosome in the dict in every iteration.
                    if( seq_dict && old_pos > seq_dict->get( chr ).length ) {
                        throw std::invalid_argument(
                            "In ChromosomeWindowStream: Chromosome \"" + chr + "\" has length " +
                            std::to_string( seq_dict->get( chr ).length ) +
                            " in the provided sequence dictionary or reference genome, "
                            "but the input data contains positions up to " +
                            std::to_string( old_pos ) + " for that chromosome."
                        );
                    }

                    // If we have a ref genome, we use that for the window positions.
                    // If not (here), we use the last position we found in the input.
                    if( ! seq_dict ) {
                        window.last_position( old_pos );
                    }

                    // We are done with the chromosome (or whole input), and signal this via nullptr.
                    return nullptr;
                }
                assert( cur != end );
                assert( par->chromosome_function( *cur ) == chr );

                // Check that it is in the correct order.
                auto const new_pos = par->position_function( *cur );
                if( old_pos >= new_pos ) {
                    throw std::runtime_error(
                        "Invalid order on chromosome " + chr + " with position " +
                        std::to_string( old_pos ) + " followed by position " +
                        std::to_string( new_pos )
                    );
                }

                // Return a pointer to the element.
                return &*cur;
            };
        }

        value_type& get_current_window_() const override final
        {
            return const_cast<value_type&>( window_ );
        }

        base_type const* get_parent_() const override final
        {
            return parent_;
        }

    private:

        // Parent. Needs to live here to have the correct derived type.
        ChromosomeWindowStream const* parent_ = nullptr;

        // Store the iterator for the window.
        WindowViewType window_;

        // We keep track of which chromosomes we have seen yet, in order to allow random order,
        // but not repeated chromosomes.
        std::unordered_set<std::string> processed_chromosomes_;

    };

    // ======================================================================================
    //      Main Class
    // ======================================================================================

    // -------------------------------------------------------------------------
    //     Constructors and Rule of Five
    // -------------------------------------------------------------------------

    ChromosomeWindowStream(
        InputStreamIterator begin, InputStreamIterator end
    )
        : base_type( begin, end )
    {}

    virtual ~ChromosomeWindowStream() override = default;

    ChromosomeWindowStream( ChromosomeWindowStream const& ) = default;
    ChromosomeWindowStream( ChromosomeWindowStream&& )      = default;

    ChromosomeWindowStream& operator= ( ChromosomeWindowStream const& ) = default;
    ChromosomeWindowStream& operator= ( ChromosomeWindowStream&& )      = default;

    friend DerivedIterator;

    // -------------------------------------------------------------------------
    //     Settings
    // -------------------------------------------------------------------------

    /**
     * @brief Get the currently set sequence dictionary used for the chromosome lengths.
     */
    std::shared_ptr<genesis::sequence::SequenceDict> sequence_dict() const
    {
        return sequence_dict_;
    }

    /**
     * @brief Set a sequence dictionary to be used for the chromosome lengths.
     *
     * By default, we use the chromosome positions as given in the data to set the Window
     * first and last positions.
     * When setting a @link ::genesis::sequence::SequenceDict SequenceDict@endlink here, we use
     * lengths as provided instead, throwing an exception should the dict not contain a chromosome
     * of the input.
     *
     * To un-set the dictionary, simply call this function with a `nullptr`.
     */
    self_type& sequence_dict( std::shared_ptr<genesis::sequence::SequenceDict> value )
    {
        sequence_dict_ = value;
        return *this;
    }

    // -------------------------------------------------------------------------
    //     Virtual Members
    // -------------------------------------------------------------------------

protected:

    std::unique_ptr<typename base_type::BaseIterator>
    get_begin_iterator_() override final
    {
        // Cannot use make_unique here, as the Iterator constructor is private,
        // and trying to make make_unique a friend does not seem to be working...
        return std::unique_ptr<DerivedIterator>( new DerivedIterator( this ));
        // return utils::make_unique<DerivedIterator>( this );
    }

    std::unique_ptr<typename base_type::BaseIterator>
    get_end_iterator_() override final
    {
        return std::unique_ptr<DerivedIterator>( new DerivedIterator( nullptr ));
        // return utils::make_unique<DerivedIterator>( nullptr );
    }

    // -------------------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------------------

private:

    // When iterating chromosomes, we might want to look up their lengths,
    // in order to properly set the window start and end. Otherwise we use what's in the data.
    std::shared_ptr<genesis::sequence::SequenceDict> sequence_dict_;

};

// =================================================================================================
//     Make Chromosome Window View Iterator
// =================================================================================================

/**
 * @brief Helper function to instantiate a ChromosomeWindowStream for each chromosome,
 * without the need to specify the template parameters manually.
 */
template<class InputStreamIterator, class DataType = typename InputStreamIterator::value_type>
ChromosomeWindowStream<InputStreamIterator, DataType>
make_chromosome_window_stream(
    InputStreamIterator begin, InputStreamIterator end
) {
    return ChromosomeWindowStream<InputStreamIterator, DataType>( begin, end );
}

/**
 * @brief Helper function to instantiate a ChromosomeWindowStream for each chromosome,
 * for a default use case.
 *
 * This helper assumes that the underlying type of the input data stream and of the data
 * that we are sliding over are of the same type, that is, we do no conversion in the
 * `entry_input_function` functor of the ChromosomeWindowStream. It further assumes that this
 * data type has public member variables `chromosome` and `position` that are accessed by the
 * `chromosome_function` and `position_function` functors of the ChromosomeWindowStream.
 * For example, a data type that this works for is Variant data.
 */
template<class InputStreamIterator>
ChromosomeWindowStream<InputStreamIterator>
make_default_chromosome_window_stream(
    InputStreamIterator begin, InputStreamIterator end
) {
    using DataType = typename InputStreamIterator::value_type;

    // Set functors.
    auto it = ChromosomeWindowStream<InputStreamIterator>( begin, end );
    it.entry_input_function = []( DataType const& variant ) {
        return variant;
    };
    it.chromosome_function = []( DataType const& variant ) {
        return variant.chromosome;
    };
    it.position_function = []( DataType const& variant ) {
        return variant.position;
    };

    // Set properties and return.
    return it;
}

} // namespace population
} // namespace genesis

#endif // include guard
