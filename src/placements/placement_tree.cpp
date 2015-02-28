/**
 * @brief Implementation of PlacementTree class.
 *
 * @file
 * @ingroup placement
 */

#include "placements/placement_tree.hpp"

#include <map>

#include "placements/placements.hpp"

namespace genesis {

// =============================================================================
//     PlacementEdgeData
// =============================================================================

/**
 * @brief Returns the number of placements on this edge.
 */
size_t PlacementEdgeData::PlacementCount() const
{
    return placements.size();
}

/**
 * @brief Returns the mass of the placements on this edge, as given by their `like_weight_ratio`.
 */
double PlacementEdgeData::PlacementMass() const
{
    double mass = 0.0;
    for (PqueryPlacement pl : placements) {
        mass += pl.like_weight_ratio;
    }
    return mass;
}

/**
 * @brief Sorts the placements on this edge by their `distal_length`.
 */
void PlacementEdgeData::SortPlacements()
{
    std::multimap<double, PqueryPlacement*> sorted;
    std::vector<PqueryPlacement*> new_placements;

    for (PqueryPlacement* place : placements) {
        sorted.emplace(place->distal_length, place);
    }
    std::multimap<double, PqueryPlacement*>::iterator it;
    for (it = sorted.begin(); it != sorted.end(); ++it) {
        new_placements.push_back(it->second);
    }

    placements.swap(new_placements);
}

} // namespace genesis