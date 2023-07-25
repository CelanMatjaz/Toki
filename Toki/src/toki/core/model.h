#pragma once

#include "toki.h"

namespace Toki {

    struct Vertex {
        glm::vec3 position{};
        glm::vec2 uv{};
        glm::vec3 normal{};

        bool operator==(const Vertex& other) const {
            return position == other.position /* && color == other.color */ && normal == other.normal && uv == other.uv;
        }

        bool operator!=(const Vertex& other) const {
            return position != other.position /* || color != other.color */ || normal != other.normal || uv != other.uv;
        }
    };

    struct InstanceData {
        alignas(glm::vec4) glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        alignas(glm::vec4) glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
        alignas(glm::vec4) glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
        alignas(glm::vec4) glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

        InstanceData(
            glm::vec3 position = { 0.0f, 0.0f, 0.0f },
            glm::vec3 rotation = { 0.0f, 0.0f, 0.0f },
            glm::vec3 scale = { 1.0f, 1.0f, 1.0f },
            glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }
        ) : position(position), rotation(rotation), scale(scale), color(color) {}
    };

    class Model {
    public:
        Model();
        ~Model();

        void loadModelData(void* vertexData, uint32_t vertexDataSize, void* indexData, uint32_t indexDataSize);
        void loadModelFromObj(std::filesystem::path path);
        InstanceData* addInstance(const InstanceData& instanceData);
        void setInstances(VulkanBufferData* data);
        void draw(VkCommandBuffer cmd);

        InstanceData* getInstances() { return instances; }
        uint32_t getNumberOfInstances() { return nInstances; }

    private:
        bool isLoaded = false;
        TkRef<VulkanBuffer> vertexBuffer;
        TkRef<VulkanBuffer> indexBuffer;
        TkRef<VulkanBuffer> instanceBuffer;

        uint32_t nInstances = 0;
        uint32_t nIndexes = 0;

        InstanceData* instances;

        inline static uint32_t maxInstances = 32 * 32;
    };

}