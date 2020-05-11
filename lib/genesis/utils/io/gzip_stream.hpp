#ifndef GENESIS_UTILS_IO_GZIP_STREAM_H_
#define GENESIS_UTILS_IO_GZIP_STREAM_H_

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

/*
    The code in this file as well as the according header file lib/utils/io/gzip_stream.hpp are
    adapted from the excellent zstr library (C++ header-only ZLib wrapper" classes) by Matei David,
    see https://github.com/mateidavid/zstr

    We adapted the original code by renaming all classes and variables to our standards,
    moving much of the implementation into a source file (so that the header does not clutter
    its callers with zlib-internal symbols), and refining some functionality.

    For this and the according source file, we need to include the following original license:

    The MIT License (MIT)

    Copyright (c) 2015 Matei David, Ontario Institute for Cancer Research

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

/**
 * @brief
 *
 * @file
 * @ingroup utils
 */

#include "genesis/utils/io/gzip.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

namespace genesis {
namespace utils {

// ================================================================================================
//     Gzip Compression Levels
// ================================================================================================

/**
 * @brief List of possible compression levels used for GzipOStream.
 *
 * The compression levels are handed over to zlib for compression, which currently allows all values
 * between 1 (best speed) and 9 (best compression), with the special case 0 (no compression), as
 * well as -1 for the default compression. Currently, the zlib default compression level corresponds
 * to level 6, as this is a good compromise between speed and compression
 * (it forms the "elbow" of the curve), hence we also use this as our default level.
 *
 * The enum only lists those four special levels. However, we use a fixed enum here (with the
 * underlying type `int`), meaning that all values in between 1 and 9 are also allowed to be used.
 * Values outside of the range [-1, 9] will lead to an exception being thrown when used in GzipOStream.
 */
enum class GzipCompressionLevel : int
{
    kDefaultCompression = -1,
    kNoCompression      =  0,
    kBestSpeed          =  1,
    kBestCompression    =  9
};

// ================================================================================================
//     Gzip Input Stream
// ================================================================================================

/**
 * @brief Input stream that offers on-the-fly gzip-decompression if needed.
 *
 * The class accesses an external std::streambuf. It can be constructed from an existing
 * std::istream (such as std::cin) or std::streambuf.
 *
 * If @p auto_detect is `true` (default), the class seamlessly auto-detects whether the source
 * stream is compressed or not. The following compressed streams are detected:
 *
 *   * GZip header, when stream starts with `1F 8B`. See [GZip format](http://en.wikipedia.org/wiki/Gzip).
 *   * ZLib header, when stream starts with `78 01`, `78 9C`, or `78 DA`. See [answer here](http://stackoverflow.com/a/17176881).
 *
 * If none of these formats are detected, the class assumes the input is not compressed,
 * and it produces a plain copy of the source stream.
 *
 * The class is based on the zstr::istream class of the excellent
 * [zstr library](https://github.com/mateidavid/zstr) by Matei David; see also our
 * @link supplement_acknowledgements_code_reuse_gzip_streams Acknowledgements@endlink.
 *
 * If genesis is compiled without zlib support, constructing an instance of this class will
 * throw an exception.
 */
class GzipIStream
    : public std::istream
{
public:

    GzipIStream(
        std::istream& is,
        bool auto_detect = true,
        std::size_t buffer_size = (std::size_t) 1 << 20
    );

    explicit GzipIStream(
        std::streambuf * sbuf_p,
        bool auto_detect = true,
        std::size_t buffer_size = (std::size_t) 1 << 20
    );

    virtual ~GzipIStream();
};

// ================================================================================================
//     Gzip Output Stream
// ================================================================================================

/**
 * @brief Output stream that offers on-the-fly gzip-compression.
 *
 * The class accesses an external std::streambuf. It can be constructed from an existing
 * std::ostream (such as std::cout) or std::streambuf.
 *
 * The GzipOStream destructor flushes all reamining data to the target ostream. However, if the
 * ostream needs to be accessed before the GzipOStream is destroyed (e.g., goes out of scope),
 * the GzipOStream::flush() function can be called manually.
 *
 * The class is based on the zstr::ostream class of the excellent
 * [zstr library](https://github.com/mateidavid/zstr) by Matei David; see also our
 * @link supplement_acknowledgements_code_reuse_gzip_streams Acknowledgements@endlink.
 *
 * If genesis is compiled without zlib support, constructing an instance of this class will
 * throw an exception.
 */
class GzipOStream
    : public std::ostream
{
public:

    GzipOStream(
        std::ostream& os,
        GzipCompressionLevel level = GzipCompressionLevel::kDefaultCompression,
        std::size_t buffer_size = (std::size_t) 1 << 20
    );

    explicit GzipOStream(
        std::streambuf* sbuf_p,
        GzipCompressionLevel level = GzipCompressionLevel::kDefaultCompression,
        std::size_t buffer_size = (std::size_t) 1 << 20
    );

    virtual ~GzipOStream();
};

} // namespace utils
} // namespace genesis

#endif // include guard
