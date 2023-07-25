#pragma once
#include "toki/core/toki.h"



#include "random"


class ProgLayer : public Toki::Layer {
public:
    static inline const uint32_t MAX_LIGHTS = 8;

    struct Light {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec4 color;
    };

    struct LightUniform {
        Light lights[MAX_LIGHTS];
        alignas(4) uint32_t lightCount;
        alignas(4) float ambientLight;
    };

    struct PushConstant {
        glm::mat4 mvp;
    };

    ProgLayer();
    ~ProgLayer();

    void onImGuiUpdate(float deltaTime) override;
    void onUpdate(float deltaTime) override;
    void onEvent(Toki::Event& e) override;

private:
    void resetInstances(uint32_t nX, uint32_t nY, float distanceX, float distanceY, bool randomParams = false);

    // Pipeline data
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    // Textures
    Toki::TkRef<Toki::VulkanTexture> noTexture;
    Toki::TkRef<Toki::VulkanTexture> testTexture;

    // Uniform data
    VkDescriptorSet descriptorSetLights;
    VkDescriptorSet descriptorSetTextures;
    VkSampler sampler;

    // Light Uniform data
    LightUniform* light;
    Toki::VulkanBufferSpecification lightUniformSpec{ sizeof(LightUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    Toki::TkRef<Toki::VulkanBuffer> lightUniform;

    // Shown instances
    Toki::InstanceData* instances = nullptr;
    int nInstancesX = 1;
    int nInstancesY = 1;
    bool randomParams = false;

    // Model
    Toki::Model loadedModel;

    // Misc
    VkPhysicalDeviceProperties properties{};
    float speed = 10.0f; // Camera speed
    bool cursorInFocus = false;
    Toki::Camera camera{ Toki::Camera::CameraProjection::PERSPECTIVE, Toki::Camera::CameraType::FIRST_PERSON, { 0.0f, 0.0f, 0.0f } };
};