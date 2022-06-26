#include "lve_model.hpp"


namespace lve {
    LveModel::LveModel(LveDevice &device, const std::vector<Vertex> &vertices) : lve_device_(device){
        createVertexBuffers(vertices);
    }

    LveModel::~LveModel() {
        // Alloc Callback field can be fed a allocator struct to help manage the memory.
        // for alloc, free, and realloc.
        // For now using nullptr, and managing Vkbuffer and Vkmemory manually.
        // Problem: There exist hard limit to number of active allocation(~1000) different for different GPUs.
        // Solution: Allocate bigger chunks of memory and assign different regions to different resources.
        vkDestroyBuffer(lve_device_.device(), vertex_buffer_, /*alloc callback*/ nullptr);
        vkFreeMemory(lve_device_.device(), vertex_buffer_memory_, /*alloc callback*/ nullptr);
    }

    void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertex_count_ = vertices.size();
        assert(vertex_count_ >= 3 && "Vertex count must at least be 3 to form a triangle.");
        VkDeviceSize buffer_size = vertex_count_ * sizeof(vertices[0]);
        // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT => Using data for vertex shader input.
        // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT => Using data for vertex shader input.
        // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT => want allocated memory to be accesible from host.
        // S.T Host can write to device memory.
        // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT => Automatically sync data in host memory and device memory. i.e no need to memcpy,
        // or to 'VkFlush', it automatically does it for you.
        lve_device_.createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            vertex_buffer_,
            vertex_buffer_memory_
        );
        void *data;
        vkMapMemory(lve_device_.device(), vertex_buffer_memory_, 0, buffer_size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(lve_device_.device(), vertex_buffer_memory_);
    }

    void LveModel::bind(VkCommandBuffer command_buffer){
        VkBuffer buffers[] = {vertex_buffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer,/*firstBinding*/ 0, /*bindingCount*/ 1, buffers, offsets);
    }

    void LveModel::draw(VkCommandBuffer command_buffer){
        vkCmdDraw(command_buffer, vertex_count_, /*num_of_instance*/ 1, /*first_vertex*/ 0, /*first_instance*/ 0);
    }

    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> binding_description(1);
        // binding_id being used.
        binding_description[0].binding = 0;
        binding_description[0].stride = sizeof(Vertex);
        // specifying whether vertex attribute addressing is a function of the vertex index or of the instance index.
        binding_description[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_description;
    }
    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attribute_description(1);
        attribute_description[0].location = 0;
        attribute_description[0].binding = 0;
        attribute_description[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_description[0].offset = 0;
        return attribute_description;
    }


}