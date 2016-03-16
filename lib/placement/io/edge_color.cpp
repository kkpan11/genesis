/**
 * @brief
 *
 * @file
 * @ingroup placement
 */

#include "placement/io/edge_color.hpp"

#include "placement/function/helper.hpp"
#include "placement/function/functions.hpp"
#include "placement/placement_tree.hpp"
#include "placement/sample.hpp"
#include "utils/tools/color.hpp"
#include "utils/tools/color/gradient.hpp"

#include <cmath>

namespace genesis {
namespace placement {

// =================================================================================================
//     Placement Edge Color Functions
// =================================================================================================

/**
 * @brief Returns a vector with a Color for each edge that visualizes the number of placements on
 * that edge.
 *
 * The vector is indexed using the edge.index(). Each edge gets assigned a Color value with these
 * properties:
 *
 *   * Edges with no placements on them are grey (RGB 128, 128, 128).
 *   * Edges with placements get a color according to the relative number of placements compared to
 *     the other edges. The edge with most placements is pure red (RGB 255, 0, 0), while lower
 *     numbers of placements smoothly transition towards yellow and green edges.
 *
 * The gradient can be controlled via the `linear` parameter. If set to `true`, the scaling of the
 * color gradient is linar in the number of placements. If set to `false` (default), it is
 * logarithmic. This way, the color resolution is higher for low placement numbers, and compressed
 * for higher numbers. A typical distribution of placements yields only some edges with a very high
 * number of placements, while most of the other edges have little to no placements. Thus, it is
 * reasonable to emphasize the differences between those edges with a lower placement count - which
 * is what the default does.
 *
 * See color heat_gradient() for more information.
 */
std::vector<utils::Color> placement_color_count_gradient( Sample const& smp, bool linear )
{
    // Init the result vector with grey color for each edge.
    auto ret = std::vector<utils::Color>( smp.tree().edge_count(), utils::Color(128,128,128) );

    // Get the highest number of placements on any edge.
    // If this is zero, there are no placements, so we can immediately return.
    auto const max_placements_per_edge = placement_count_max_edge(smp).second;
    if (max_placements_per_edge == 0) {
        return ret;
    }

    auto place_map = placements_per_edge( smp );

    // Calculate the heat gradient color based on the number of placements for each edge.
    for (auto it = smp.tree().begin_edges(); it != smp.tree().end_edges(); ++it) {
        auto const& edge = **it;
        auto const placements_on_edge = place_map[ edge.index() ].size();

        if( placements_on_edge > 0) {
            double val;
            if (linear) {
                val = placements_on_edge / max_placements_per_edge;
            } else {
                val = log( placements_on_edge ) / log( max_placements_per_edge );
            }
            ret[edge.index()] = utils::heat_gradient(val);
        }

        // LOG_DBG <<  edge.placements.size() << " --> "
        //         << 100 * edge.placements.size() / max_placements_per_edge_
        //         << " = " << edge_color.dump();
    }

    return ret;
}

} // namespace placement
} // namespace genesis
