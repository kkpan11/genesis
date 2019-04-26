#ifndef GENESIS_UTILS_CONTAINERS_DATAFRAME_READER_H_
#define GENESIS_UTILS_CONTAINERS_DATAFRAME_READER_H_

/*
    Genesis - A toolkit for working with phylogenetic data.
    Copyright (C) 2014-2019 Lucas Czech and HITS gGmbH

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

#include "genesis/utils/containers/dataframe.hpp"
#include "genesis/utils/core/std.hpp"
#include "genesis/utils/formats/csv/reader.hpp"
#include "genesis/utils/io/input_source.hpp"
#include "genesis/utils/io/input_stream.hpp"

#include <functional>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

namespace genesis {
namespace utils {

// =================================================================================================
//     DataframeReader
// =================================================================================================

template<typename T = std::string>
class DataframeReader
{
public:

    // -------------------------------------------------------------
    //     Constructors and Rule of Five
    // -------------------------------------------------------------

    explicit DataframeReader( char separator_char = ',' )
    {
        reader_.separator_chars( std::string( 1, separator_char ));
    }

    explicit DataframeReader( CsvReader const& reader )
        : reader_( reader )
    {}

    ~DataframeReader() = default;

    DataframeReader(DataframeReader const&) = default;
    DataframeReader(DataframeReader&&)      = default;

    DataframeReader& operator= (DataframeReader const&) = default;
    DataframeReader& operator= (DataframeReader&&)      = default;

    // -------------------------------------------------------------
    //     Reading
    // -------------------------------------------------------------

    Dataframe read(
        std::shared_ptr<BaseInputSource> source
    ) const {
        utils::InputStream is( source );
        return parse_( is );
    }

    // -------------------------------------------------------------
    //     Properties
    // -------------------------------------------------------------

    bool col_names_from_first_row() const
    {
        return col_names_from_first_row_;
    }

    bool row_names_from_first_col() const
    {
        return row_names_from_first_col_;
    }

    DataframeReader& col_names_from_first_row( bool value )
    {
        col_names_from_first_row_ = value;
        return *this;
    }

    DataframeReader& row_names_from_first_col( bool value )
    {
        row_names_from_first_col_ = value;
        return *this;
    }

    CsvReader& csv_reader()
    {
        return reader_;
    }

    CsvReader const& csv_reader() const
    {
        return reader_;
    }

    DataframeReader& parse_value_functor( std::function<T( std::string const& )> functor )
    {
        parse_value_ = functor;
        return *this;
    }

    // -------------------------------------------------------------
    //     Internal Functions
    // -------------------------------------------------------------

private:

    Dataframe parse_(
        utils::InputStream& input_stream
    ) const {
        Dataframe result;
        size_t const offset = ( row_names_from_first_col_ ? 1 : 0 );
        size_t line_cnt = 0;

        // Early stop.
        if( ! input_stream ) {
            return result;
        }

        // Read column names.
        if( col_names_from_first_row_ ) {
            auto const col_names = reader_.parse_line( input_stream );
            ++line_cnt;

            size_t const start = offset;
            for( size_t i = start; i < col_names.size(); ++i ) {
                result.add_col<T>( col_names[i] );
            }
        }

        // Read lines of data.
        while( input_stream ) {
            auto const line = reader_.parse_line( input_stream );
            ++line_cnt;

            // Need to have a least one content element.
            if(( line.size() == 0 ) || ( row_names_from_first_col_ && line.size() == 1 )) {
                throw std::runtime_error(
                    "Cannot read Dataframe with lines that do not contain any content (line " +
                    std::to_string( line_cnt ) + "). Maybe the separator char is wrong."
                );
            }
            assert( line.size() > offset );

            // Add a row for the line. Use row name if wanted.
            if( row_names_from_first_col_ ) {
                result.add_row( line[0] );
            } else {
                result.add_unnamed_row();
            }

            // If there was no column names, make columns.
            if( result.cols() == 0 ) {
                // This can only happen in the first line, and if no col names were read.
                assert( result.rows() == 1 );
                assert( ! col_names_from_first_row_ );

                // Add unnamed cols.
                for( size_t i = offset; i < line.size(); ++i ) {
                    result.add_unnamed_col<T>();
                }
                assert( line.size() == offset + result.cols() );
            }

            // Check if the line has the correct size.
            if( line.size() != offset + result.cols() ) {
                throw std::runtime_error(
                    "Dataframe input has different line lengths (line " +
                    std::to_string( line_cnt ) + ")."
                );
            }

            // Parse and transfer the data. User specified parser or default one.
            auto const row_idx = result.rows() - 1;
            if( parse_value_ ) {
                for( size_t i = 0; i < result.cols(); ++i ) {
                    // result( row_idx, i ) = parse_value_( line[ offset + i ] );
                    // result[i][row_idx] = parse_value_( line[ offset + i ] );
                    auto& col = dynamic_cast<Dataframe::Column<T>&>(result[i]);
                    col[row_idx] = parse_value_( line[ offset + i ] );
                }
            } else {
                for( size_t i = 0; i < result.cols(); ++i ) {
                    // result( row_idx, i ) = parse_value_stringstream_( line[ offset + i ] );
                    // result[i][row_idx] = parse_value_stringstream_( line[ offset + i ] );
                    auto& col = dynamic_cast<Dataframe::Column<T>&>(result[i]);
                    col[row_idx] = parse_value_stringstream_( line[ offset + i ] );
                }
            }
        }

        assert( result.rows() == line_cnt - ( col_names_from_first_row_ ? 1 : 0 ));
        return result;
    }

    inline T parse_value_stringstream_( std::string const& cell ) const
    {
        std::stringstream ss( cell );
        T value;
        ss >> value;
        return value;
    }

    // -------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------

private:

    bool col_names_from_first_row_ = true;
    bool row_names_from_first_col_ = true;

    CsvReader reader_;

    std::function<T( std::string const& )> parse_value_;

};

} // namespace utils
} // namespace genesis

#endif // include guard
