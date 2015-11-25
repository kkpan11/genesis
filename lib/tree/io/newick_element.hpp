#ifndef GENESIS_TREE_IO_NEWICK_ELEMENT_H_
#define GENESIS_TREE_IO_NEWICK_ELEMENT_H_

/**
 * @brief
 *
 * @file
 * @ingroup tree
 */

#include <stdexcept>
#include <string>
#include <vector>

namespace genesis {

// =================================================================================================
//     Forward Declarations
// =================================================================================================

class NewickBroker;

// =================================================================================================
//     NewickBrokerElement
// =================================================================================================

/** @brief POD struct that stores the information for one tree node.
 *
 * Most of its members are public, as it is intended to serve an an intermediate data
 * exchange format, so different callers might need to modify its content.
 * However, this means paying attention when working with the data, as it can
 * be changed from anywhere.
 *
 * See NewickBroker class for a description of this intermediate format.
 */
struct NewickBrokerElement
{
    friend NewickBroker;

    // -------------------------------------------------------------------------
    //     Constructor and Rule of Five
    // -------------------------------------------------------------------------

public:
    /**
     * @brief Constructor, initializes the item values.
     */
    NewickBrokerElement()
        : branch_length(0.0)
        , depth(0)
        , is_leaf(false)
        , rank_(-1)
    {}

    ~NewickBrokerElement() = default;

    NewickBrokerElement(NewickBrokerElement const&) = default;
    NewickBrokerElement(NewickBrokerElement&&)      = default;

    NewickBrokerElement& operator= (NewickBrokerElement const&) = default;
    NewickBrokerElement& operator= (NewickBrokerElement&&)      = default;

    // -------------------------------------------------------------------------
    //     Public Data Members
    // -------------------------------------------------------------------------

    /**
     * @brief Name of the node.
     *
     * In case it is a leaf, this is usually the name of the taxon represented by the node.
     * Internal nodes are named "Internal Node" in case no name is specified in the Newick format,
     * same applies to the (possibly virtual) root, which is named "Root Node" by default.
     */
    std::string name;

    /**
     * @brief Branch length associated with the node, i.e. the edge leading to its parent.
     */
    double      branch_length;

    /**
     * @brief Depth of the node in the tree, i.e. its distance from the root.
     */
    int         depth;

    /**
     * @brief True if the node is a leaf/tip, false otherwise.
     *
     * This value can be set for example while parsing a Newick tree, and is used by
     * NewickBroker::validate() as a check for correctness. However, it is (so far) not used further,
     * because it is not reliable (it can be changed arbitrarily without checking whether this is
     * conform with the tree topology).
     *
     * Therefore, use rank() to check whether a node is a leaf (in order to use rank, first
     * NewickBroker::assign_ranks() has to be called).
     */
    bool        is_leaf;

    /**
     * @brief Arbitrary strings that can be attached to a node, e.g. in Newick format via "{}".
     */
    std::vector<std::string> tags;

    /**
     * @brief Arbitrary strings that can be attached to a node, e.g. in Newick format via "[]".
     */
    std::vector<std::string> comments;

    // -------------------------------------------------------------------------
    //     Additional Members
    // -------------------------------------------------------------------------

    /**
     * @brief Returns the rank (number of immediate children) of this node.
     *
     * NewickBroker::assign_ranks() has to be called before using this function. Otherwise,
     * this function will throw an std::logic_error.
     */
    int rank() const
    {
        if (rank_ < 0) {
            throw std::logic_error("NewickBroker::assign_ranks() was not called before.");
        }
        return rank_;
    }

private:

    /**
     * @brief Rank of the node, i.e. how many children it has.
     */
    mutable int rank_;
};

} // namespace genesis

#endif // include guard
