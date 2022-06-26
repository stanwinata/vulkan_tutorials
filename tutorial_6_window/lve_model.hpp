#pragma once

#include "lve_device.hpp"

// To ensure it's in radians not in degrees.
#define GLM_FORCE_RADIANS

// GLM will expect depth to be [0,1].
// Unlike openGL which is [-1,1].
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve {
    // This class is utilized to take vertex data created by
    // or read from a file on the cpu. Then allocate + copy data into device GPU.
    class LveModel {
        public:
            struct Vertex {
                glm::vec2 position_;

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };
            LveModel(LveDevice &device, const std::vector<Vertex> &vertices);
            ~LveModel();
            // Need to remove copy constructor because
            // LveModel manages vulkan buffers and memory objects.
            LveModel(const LveModel &) = delete;
            LveModel &operator=(const LveModel &) = delete;

            // Binds vertex buffer/input data to command_buffer.
            void bind(VkCommandBuffer command_buffer);
            // Call commandbuffer to draw.
            void draw(VkCommandBuffer command_buffer);
        private:
            // Allocate memory and buffer in CPU+Device + set data to the given input.
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            LveDevice &lve_device_;
            // In Vulkan, buffer and assigned memory are separate.
            // contrast to memory being allocated automatically assigned for buffer.
            // gives programmer more control for memory management.
            VkBuffer vertex_buffer_;
            VkDeviceMemory vertex_buffer_memory_;
            uint32_t vertex_count_;
    };
}