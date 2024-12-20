#ifndef GENESIS_TAXONOMY_PRINTERS_NESTED_H_
#define GENESIS_TAXONOMY_PRINTERS_NESTED_H_

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
 * @ingroup taxonomy
 */

#include <iosfwd>
#include <string>

namespace genesis {
namespace taxonomy {

// =================================================================================================
//     Forward Declarations
// =================================================================================================

class Taxonomy;

// =================================================================================================
//     PrinterNested
// =================================================================================================

/**
 * @brief Simple printer class for Taxonomy.
 */
class PrinterNested
{
public:

    // -------------------------------------------------------------------------
    //     Constructors and Rule of Five
    // -------------------------------------------------------------------------

    PrinterNested()  = default;
    ~PrinterNested() = default;

    PrinterNested( PrinterNested const& ) = default;
    PrinterNested( PrinterNested&& )      = default;

    PrinterNested& operator= ( PrinterNested const& ) = default;
    PrinterNested& operator= ( PrinterNested&& )      = default;

    // -------------------------------------------------------------------------
    //     Print
    // -------------------------------------------------------------------------

    void print(
        std::ostream&   out,
        Taxonomy const& tax
    ) const;

    std::string print(
        Taxonomy const& tax
    ) const;

    std::string operator() (
        Taxonomy const& tax
    ) const;

    // -------------------------------------------------------------------------
    //     Properties
    // -------------------------------------------------------------------------

    PrinterNested& line_limit( int value );
    int            line_limit() const;

    PrinterNested& depth_limit( int value );
    int            depth_limit() const;

    PrinterNested& indent_string( std::string value );
    std::string    indent_string() const;

    PrinterNested& print_ranks( bool value );
    bool           print_ranks() const;

    PrinterNested& print_ids( bool value );
    bool           print_ids() const;

    PrinterNested& print_sizes( bool value );
    bool           print_sizes() const;

    // -------------------------------------------------------------------------
    //     Internal Functions
    // -------------------------------------------------------------------------

private:

    bool print_to_ostream_(
        std::ostream&   out,
        Taxonomy const& tax,
        size_t          depth,
        size_t&         lines
    ) const;

    // -------------------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------------------

private:

    int line_limit_  = -1;
    int depth_limit_ = -1;
    std::string indent_string_ = "\t";

    bool print_ranks_ = true;
    bool print_ids_ = true;
    bool print_sizes_ = true;

};

} // namespace taxonomy
} // namespace genesis

#endif // include guard
