#include "tkpch.h"
#include "model.h"

#include "toki/assets/load_obj.h"

namespace Toki {

    Model::Model() {
        instances = new InstanceData[maxInstances]();
        Toki::VulkanBufferSpecification instanceBufferSpec{ maxInstances * sizeof(InstanceData), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
        instanceBuffer = VulkanBuffer::create(&instanceBufferSpec);
        Toki::InstanceData singleInstance = { { 0.f, 0, 0.f } };
        VulkanBufferData data1(sizeof(InstanceData), &singleInstance);
        instanceBuffer->setData(&data1);
    }

    Model::~Model() {
        delete[] instances;
    }

    void Model::loadModelData(void* vertexData, uint32_t vertexDataSize, void* indexData, uint32_t indexDataSize) {
        Toki::VulkanBufferSpecification vertexBufferSpec{ vertexDataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
        vertexBuffer = VulkanBuffer::create(&vertexBufferSpec);
        VulkanBufferData vertexDataA(vertexDataSize, vertexData);
        vertexBuffer->setData(&vertexDataA);
        Toki::VulkanBufferSpecification indexBufferSpec{ indexDataSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
        indexBuffer = VulkanBuffer::create(&indexBufferSpec);
        VulkanBufferData indexDataA(indexDataSize, indexData);
        indexBuffer->setData(&indexDataA);
        nIndexes = indexDataSize / sizeof(uint32_t);
    }

    void Model::loadModelFromObj(std::filesystem::path path) {
        const std::string pathToFile = path.string();
        GeometryData geometry = loadObj(pathToFile.c_str());

        loadModelData(
            geometry.vertexData.data(),
            geometry.vertexData.size() * sizeof(Vertex),
            geometry.indexData.data(),
            geometry.indexData.size() * sizeof(uint32_t)
        );
    }

    void Model::setInstances(VulkanBufferData* data) {
        instanceBuffer->setData(data);
        nInstances = data->size / sizeof(InstanceData);
    }

    InstanceData* Model::addInstance(const InstanceData& instanceData) {
        TK_ASSERT(nInstances < maxInstances);

        instances[nInstances] = instanceData;
        ++nInstances;
        return &instances[nInstances - 1];
    }

    void Model::draw(VkCommandBuffer cmd) {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkBuffer instances[] = { instanceBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
        vkCmdBindVertexBuffers(cmd, 1, 1, instances, offsets);
        vkCmdBindIndexBuffer(cmd, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, nIndexes, nInstances, 0, 0, 0);
    }
}