#include "model.h"

namespace Toki {

    Model::Model() {
        instances = new InstanceData[maxInstances];
        instanceBuffer = VulkanBuffer::createVertexBuffer(sizeof(InstanceData) * maxInstances);
    }

    Model::~Model() {
        // instanceBuffer->cleanup();
        // indexBuffer->cleanup();
        // vertexBuffer->cleanup();

        delete[] instances;
    }

    void Model::loadModelData(void* vertexData, uint32_t vertexDataSize, void* indexData, uint32_t indexDataSize) {
        vertexBuffer = VulkanBuffer::createVertexBuffer(vertexDataSize, vertexData);
        indexBuffer = VulkanBuffer::createIndexBuffer(indexDataSize, indexData);
        nIndexes = indexDataSize / sizeof(uint32_t);
    }

    void Model::loadModelFromObj(std::filesystem::path path) {

    }

    InstanceData* Model::addInstance(const InstanceData* instanceData) {
        TK_ASSERT(nInstances < maxInstances - 1);
        memcpy(&instances[nInstances], instanceData, sizeof(InstanceData));
        ++nInstances;

        instanceBuffer->setData(sizeof(InstanceData) * nInstances, instances);

        return &instances[nInstances - 1];
    }

    void Model::draw(VkCommandBuffer cmd) {
        instanceBuffer->setData(sizeof(InstanceData) * nInstances, instances);
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkBuffer instances[] = { instanceBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
        vkCmdBindVertexBuffers(cmd, 1, 1, instances, offsets);
        vkCmdBindIndexBuffer(cmd, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(cmd, nIndexes, nInstances, 0, 0, 0);
    }

}