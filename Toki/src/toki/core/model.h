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
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
        glm::vec4 color;

        void print() {
            std::cout << std::format("position: {} {} {}\n", position.x, position.y, position.z);
            std::cout << std::format("scale:    {} {} {}\n", scale.x, scale.y, scale.z);
            std::cout << std::format("rotation: {} {} {}\n", rotation.x, rotation.y, rotation.z);
        }
    };

    class Model {
    public:
        Model();
        ~Model();

        void loadModelData(void* vertexData, uint32_t vertexDataSize, void* indexData, uint32_t indexDataSize);
        void loadModelFromObj(std::filesystem::path path);
        InstanceData* addInstance(const InstanceData* instanceData);
        void draw(VkCommandBuffer cmd);

        InstanceData* getInstances() { return instances; }
        uint32_t getNumberOfInstances() { return nInstances; }

    private:
        bool isLoaded = false;
        std::shared_ptr<Toki::VulkanBuffer> vertexBuffer;
        std::shared_ptr<Toki::VulkanBuffer> indexBuffer;
        std::shared_ptr<Toki::VulkanBuffer> instanceBuffer;

        uint32_t nInstances = 0;
        uint32_t nIndexes = 0;

        InstanceData* instances;

        inline static uint32_t maxInstances = 256;
    };

}