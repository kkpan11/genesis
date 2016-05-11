#ifndef GENESIS_UTILS_CORE_INDEXED_LIST_H_
#define GENESIS_UTILS_CORE_INDEXED_LIST_H_

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
 * @brief Header of IndexedList class.
 *
 * @file
 * @ingroup utils
 */

#include <memory>
#include <vector>

#include "utils/core/std.hpp"

namespace genesis {
namespace utils {

// =================================================================================================
//     Forward Declarations
// =================================================================================================

template <typename T>
class IndexedListIterator;

// =================================================================================================
//     Indexed List
// =================================================================================================

/**
 * @brief
 */
template <typename T>
class IndexedList
{
public:

    // -------------------------------------------------------------------------
    //     Member Types
    // -------------------------------------------------------------------------

    using value_type      = T;

    using       reference =       value_type&;
    using const_reference = const value_type&;
    using       pointer   =       value_type*;
    using const_pointer   = const value_type*;

    using       iterator  = IndexedListIterator<      value_type>;
    using const_iterator  = IndexedListIterator<const value_type>;

    using size_type       = size_t;

    // -------------------------------------------------------------------------
    //     Constructor and Rule of Five
    // -------------------------------------------------------------------------

    /**
     * @brief Default constructor.
     */
    IndexedList () {};

    /**
     * @brief Fill constructor with default initialization.
     */
    explicit IndexedList (size_type n)
        : IndexedList(n, value_type())
    {}

    /**
     * @brief Fill constructor.
     */
    IndexedList (size_type n, const value_type& val)
    {
        content_.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            content_.push_back( utils::make_unique<value_type>(val) );
        }
    }

    /*
    // range (3)
    template <class InputIterator>
    IndexedList (InputIterator first, InputIterator last);
    */

    // copy (4)
    explicit IndexedList (const IndexedList& x) = delete;

    // move (5)
    // IndexedList (IndexedList&& x);

    // initializer list (6)
    // IndexedList (std::initializer_list<value_type> il);

    // copy (1)
    IndexedList& operator= (const IndexedList& x) = delete;

    // move (2)
    // IndexedList& operator= (IndexedList&& x);

    /**
     * @brief Destructor.
     */
    ~IndexedList() = default;

    // -------------------------------------------------------------------------
    //     Iterators
    // -------------------------------------------------------------------------

    iterator begin() noexcept
    {
        return IndexedListIterator<value_type> (*this);
    }

    const_iterator begin() const noexcept
    {
        return IndexedListIterator<const value_type> (*this);
    }

    iterator end() noexcept
    {
        return IndexedListIterator<value_type> (*this, size());
    }

    const_iterator end() const noexcept
    {
        return IndexedListIterator<const value_type> (*this, size());
    }

    const_iterator cbegin() const noexcept
    {
        return IndexedListIterator<const value_type> (*this);
    }

    const_iterator cend() const noexcept
    {
        return IndexedListIterator<const value_type> (*this, size());
    }

    // -------------------------------------------------------------------------
    //     Capacity
    // -------------------------------------------------------------------------

    size_type size() const noexcept
    {
        return content_.size();
    }

    bool empty() const noexcept
    {
        return content_.empty();
    }

    // -------------------------------------------------------------------------
    //     Element Access
    // -------------------------------------------------------------------------

    reference operator[] (size_type n)
    {
        return *(content_[n]);
    }

    const_reference operator[] (size_type n) const
    {
        return *(content_[n]);
    }

    reference at (size_type n)
    {
        return *(content_.at(n));
    }

    const_reference at (size_type n) const
    {
        return *(content_.at(n));
    }

    reference front()
    {
        return *(content_.front());
    }

    const_reference front() const
    {
        return *(content_.front());
    }

    reference back()
    {
        return *(content_.back());
    }

    const_reference back() const
    {
        return *(content_.back());
    }

    // -------------------------------------------------------------------------
    //     Modifiers
    // -------------------------------------------------------------------------

    void push_back()
    {

    }

    // -------------------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------------------

private:

    std::vector<std::unique_ptr<value_type>> content_;
};

// =================================================================================================
//     Indexed List Iterator
// =================================================================================================

template <typename T>
class IndexedListIterator
{
public:

    // -------------------------------------------------------------------------
    //     Member Types
    // -------------------------------------------------------------------------

    using value_type      = T;
    using self_type       = IndexedListIterator<value_type>;

    using       reference =       value_type&;
    using const_reference = const value_type&;
    using       pointer   =       value_type*;
    using const_pointer   = const value_type*;

    using size_type       = size_t;

    // -------------------------------------------------------------------------
    //     Constructor and Rule of Five
    // -------------------------------------------------------------------------

    // IndexedListIterator()
    //     : pos_(0)
    //     , il_(il)
    // {}

    IndexedListIterator(IndexedList<value_type>& il)
        : pos_(0)
        , il_(il)
    {}

    IndexedListIterator(IndexedList<value_type>& il, size_type pos)
        : pos_(pos)
        , il_(il)
    {}

    // -------------------------------------------------------------
    //     Accessors
    // -------------------------------------------------------------

    reference operator * ()
    {
        return il_[pos_];
    }

    pointer operator -> ()
    {
        return &(il_[pos_]);
    }

    // -------------------------------------------------------------
    //     Iteration
    // -------------------------------------------------------------

    self_type& operator ++ ()
    {
        ++pos_;
        return *this;
    }

    self_type operator ++ (int)
    {
        self_type tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator == (self_type const& other) const
    {
        return &il_ == &(other.il_) && pos_ == other.pos_;
    }

    bool operator != (self_type const& other) const
    {
        return !(*this == other);
    }

    // -------------------------------------------------------------------------
    //     Data Members
    // -------------------------------------------------------------------------

private:

    size_type                pos_;
    IndexedList<value_type>& il_;
};

} // namespace utils
} // namespace genesis

#endif // include guard
