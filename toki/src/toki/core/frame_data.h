#pragma once

#include <vector>

namespace Toki {

struct FrameData {
    // Time in milliseconds since last frame
    double deltaTime = 0;

    // Total amount of time sinice application started rendering
    double totalTime = 0;

    // Frame number
    uint32_t frameNumber = 0;

    // Total draw calls recorded last frame
    uint32_t drawCallCount = 0;

    // Total count of triangles drawn last frame
    uint32_t drawnTriangleCount = 0;

    // Total count of meshes drawn last frame
    uint32_t drawnMeshCount = 0;

    // Total count of geometries drawn last frame
    uint32_t drawnGeometryCount = 0;

    // Total draw calls recorded per thread
    std::vector<uint32_t> drawCallCounts;

    // Total triangle counts per thread
    std::vector<uint32_t> drawnTriangleCounts;

    // Total drawn meshes per thread
    std::vector<uint32_t> drawnMeshCounts;
};

}  // namespace Toki
