#pragma once

#include "lve_device.hpp"
#include <string>
#include <vector>

namespace lve {
// Not part of LvePipeline class because want application layer to configure pipeline easily, and
// reuse for multiple pipelines
struct PipelineConfigInfo {
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class LvePipeline {
    public:
    LvePipeline(LveDevice &device,
                const std::string& vert_file_path,
                const std::string& frag_file_path,
                const PipelineConfigInfo& config_info);
    ~LvePipeline();
    // RAII style to prevent memory faults.
    LvePipeline(const LvePipeline&) = delete;
    void operator=(const LvePipeline&) = delete;

    // Create default pipeline configuration.
    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

    // Binds graphic pipeline into the command buffer.
    void bind(VkCommandBuffer command_buffer);

    private:
    static std::vector<char> ReadFile(const std::string& file_name);
    void CreateGraphicPipeline(const std::string& vert_file_path,
                                const std::string& frag_file_path,
                                const PipelineConfigInfo& config_info);

    // Convert spv code to vkShaderModule.
    void CreateShaderModule(const std::vector<char>& code, VkShaderModule* output_module);

    // Storing device reference. Could've been memory unsafe if device was released before pipeline was released.
    // But since we know the implicit relationship that the device will outlive the pipeline, it's unlikely to happen.
    // Also known as aggregation relationship in UML.
    LveDevice& lve_device_;
    VkPipeline graphics_pipeline_;
    VkShaderModule vert_shader_module_;
    VkShaderModule frag_shader_module_;
};
} // namespace lve