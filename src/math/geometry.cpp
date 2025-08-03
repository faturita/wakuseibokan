#include "vec3f.h"
#include <vector>
#include <limits>
#include <cmath>

#include "yamathutil.h"
#include "geometry.h"


/**
 * @brief Finds the closest point on a polygon to a given point.
 * @param p The point to check against (e.g., the vehicle's position).
 * @param vertices A vector of vertices defining the polygon in order.
 * @param out_distance [out] The calculated minimum distance to the polygon.
 * @return The coordinates of the closest point on the polygon's perimeter.
 */
Vec3f findClosestPointOnPolygon(Vec3f p, const std::vector<Vec3f>& vertices, float& out_distance)
{
    if (vertices.empty())
    {
        out_distance = std::numeric_limits<float>::max();
        return Vec3f(0, 0, 0);
    }

    float min_dist_sq = std::numeric_limits<float>::max();
    Vec3f overall_closest_point;

    // Iterate through each edge of the polygon
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        Vec3f v1 = vertices[i];
        // The next vertex, wrapping around for the last edge
        Vec3f v2 = vertices[(i + 1) % vertices.size()];

        Vec3f closest_point_on_edge = findClosestPointOnLineSegment(p, v1, v2);

        float dist_sq = (p - closest_point_on_edge).magnitudeSquared();

        if (dist_sq < min_dist_sq)
        {
            min_dist_sq = dist_sq;
            overall_closest_point = closest_point_on_edge;
        }
    }

    out_distance = sqrt(min_dist_sq);
    return overall_closest_point;
}