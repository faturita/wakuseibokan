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

/**
 * @brief Given a set of existing points and a set of candidate points, picks the candidate
 *        that is farthest away from all existing points.  Distance is computed on the XZ plane.
 *        If existingPoints is empty, returns the first candidate.
 * @param existingPoints Positions already occupied (e.g., existing dock locations).
 * @param candidates     Available positions to choose from (e.g., coastline points).
 * @return The candidate point that maximizes the minimum distance to any existing point.
 */
Vec3f findFarthestPoint(const std::vector<Vec3f>& existingPoints, const std::vector<Vec3f>& candidates)
{
    if (candidates.empty())
        return Vec3f(0, 0, 0);

    if (existingPoints.empty())
        return candidates[0];

    float bestMinDist = -1;
    size_t bestIdx = 0;

    for (size_t i = 0; i < candidates.size(); i++)
    {
        float minDist = std::numeric_limits<float>::max();
        for (size_t j = 0; j < existingPoints.size(); j++)
        {
            float dx = candidates[i][0] - existingPoints[j][0];
            float dz = candidates[i][2] - existingPoints[j][2];
            float dist = sqrt(dx * dx + dz * dz);
            if (dist < minDist)
                minDist = dist;
        }
        if (minDist > bestMinDist)
        {
            bestMinDist = minDist;
            bestIdx = i;
        }
    }

    return candidates[bestIdx];
}