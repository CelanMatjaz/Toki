#pragma once

#include <glm/glm.hpp>

namespace Toki {

using Point2D = glm::vec2;

using Rect2D = struct Rect2D {
    Point2D position;
    Point2D size;

    bool containsPoint(const Point2D& point) {
        return position.x <= point.x && position.y <= point.y && position.x + size.x >= point.x && position.y + size.y >= point.y;
    }
};

}  // namespace Toki
