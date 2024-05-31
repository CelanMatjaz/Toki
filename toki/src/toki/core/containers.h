#pragma once

#include <glm/glm.hpp>

namespace Toki {

using Point2D = glm::vec2;
using Point3D = glm::vec3;
using Color = glm::vec4;

using Rect2D = struct Rect2D {
    Point2D position;
    Point2D size;

    bool containsPoint(const Point2D& point) {
        return position.x <= point.x && position.y <= point.y && position.x + size.x >= point.x && position.y + size.y >= point.y;
    }
};

using Extent3D = glm::uvec3;

using Region3D = struct Region3D {
    Point3D position;
    Point3D extent;
};

using Region2D = struct Region2D {
    Point2D position;
    Point2D extent;
};

}  // namespace Toki
