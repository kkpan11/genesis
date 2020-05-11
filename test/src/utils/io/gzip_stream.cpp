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
 * @ingroup test
 */

#include "src/common.hpp"

#include "genesis/utils/io/gzip_stream.hpp"
#include "genesis/utils/core/fs.hpp"

#include <sstream>

using namespace genesis;
using namespace genesis::utils;

// Helper function, copied from https://github.com/mateidavid/zstr/blob/master/examples/zc.cpp
// License information: see genesis/lib/genesis/utils/io/gzip.hpp
void cat_stream(std::istream& is, std::ostream& os)
{
    const std::streamsize buff_size = 1 << 16;
    char * buff = new char [buff_size];
    while (true)
    {
        is.read(buff, buff_size);
        std::streamsize cnt = is.gcount();
        if (cnt == 0) break;
        os.write(buff, cnt);
    }
    delete [] buff;
}

TEST( GzipStream, CompressDecompress )
{
    NEEDS_TEST_DATA;

    std::string infile = environment->data_dir + "sequence/dna_10.fasta";
    auto const data = file_read( infile );
    std::string compr;
    std::string decompr;

    // Scope, so that we can re-use string names.
    {
        std::istringstream iss( data );
        std::ostringstream oss;

        // Use both gzip streams here. The input should detect that it is not compressed.
        GzipIStream gistr( iss );
        GzipOStream gostr( oss );

        // We here manually flush, because gostr does not go out of scope, so the sync
        // is not called within this scope.
        cat_stream( gistr, gostr );
        gostr.flush();
        compr = oss.str();
    }

    // Now, we should have compressed data.
    EXPECT_NE( data, compr );
    EXPECT_GT( data.size(), compr.size() );

    // Test that it is gzip by probing the magic bytes
    ASSERT_GT( compr.size(), 2 );
    unsigned char b0 = reinterpret_cast< unsigned char& >(compr[0]);
    unsigned char b1 = reinterpret_cast< unsigned char& >(compr[1]);
    EXPECT_TRUE( b0 == 0x1F && b1 == 0x8B );

    // Decopress again
    {
        std::istringstream iss( compr );
        std::ostringstream oss;
        GzipIStream gistr( iss );

        // Here, we only want the decompressing input stream, but not compress again.
        cat_stream( gistr, oss );
        decompr = oss.str();
    }

    // Test that we end up with the same that we started with
    EXPECT_EQ( data, decompr );
}
