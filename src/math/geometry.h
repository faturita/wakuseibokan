#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <vector>
#include "vec3f.h"

Vec3f findClosestPointOnPolygon(Vec3f p, const std::vector<Vec3f>& vertices, float& out_distance);

Vec3f findFarthestPoint(const std::vector<Vec3f>& existingPoints, const std::vector<Vec3f>& candidates);


#endif /* GEOMETRY_H_ */