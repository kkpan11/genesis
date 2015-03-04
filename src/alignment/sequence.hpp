#ifndef GNS_ALIGNMENT_SEQUENCE_H_
#define GNS_ALIGNMENT_SEQUENCE_H_

/**
 * @brief
 *
 * @file
 * @ingroup alignment
 */

#include <string>

namespace genesis {

class Sequence
{
public:

    // -----------------------------------------------------
    //     Constructor and Typedefs
    // -----------------------------------------------------

    typedef char SymbolType;

    Sequence (std::string l, std::string s) : label_(l), sites_(s) {}
    ~Sequence();

    // -----------------------------------------------------
    //     Accessors
    // -----------------------------------------------------

    inline std::string Label() const
    {
        return label_;
    }

    inline size_t Length() const
    {
        return sites_.size();
    }

    inline SymbolType Site(size_t index) const
    {
        return sites_[index];
    }

    inline const std::string& Sites() const
    {
        return sites_;
    }

    // -----------------------------------------------------
    //     Mutators
    // -----------------------------------------------------

    void RemoveGaps();

    // -----------------------------------------------------
    //     Dump and Debug
    // -----------------------------------------------------

    std::string Dump() const;

    // -----------------------------------------------------
    //     Members
    // -----------------------------------------------------

    SymbolType  gap_char = '-';

protected:
    std::string label_;
    std::string sites_;
};

} // namespace genesis

#endif // include guard
