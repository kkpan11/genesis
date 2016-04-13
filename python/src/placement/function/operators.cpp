/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2016 Lucas Czech

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
 * @ingroup python
 */

#include <python/src/common.hpp>

#include "lib/genesis.hpp"

using namespace ::genesis::placement;

PYTHON_EXPORT_FUNCTIONS(placement_function_operators_export, "placement")
{

    boost::python::def(
        "compatible_trees",
        ( bool ( * )( const Sample &, const Sample & ))( &::genesis::placement::compatible_trees ),
        ( boost::python::arg("lhs"), boost::python::arg("rhs") )
    );

    boost::python::def(
        "operator<<",
        ( std::ostream & ( * )( std::ostream &, Sample const & ))( &::genesis::placement::operator<< ),
        ( boost::python::arg("out"), boost::python::arg("smp") ),
        boost::python::return_value_policy<boost::python::reference_existing_object>(),
        get_docstring("std::ostream & ::genesis::placement::operator<< (std::ostream & out, Sample const & smp)")
    );

    boost::python::def(
        "print_tree",
        ( std::string ( * )( Sample const & ))( &::genesis::placement::print_tree ),
        ( boost::python::arg("smp") ),
        get_docstring("std::string ::genesis::placement::print_tree (Sample const & smp)")
    );
}