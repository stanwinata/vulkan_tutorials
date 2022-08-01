#include "simple_renderer_system.hpp"
#include <stdexcept>
#include <array>
#include <glm/gtc/constants.hpp>

namespace lve {

struct SimplePushConstantData {
  glm::mat2 transform;
  alignas(8) glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

SimpleRendererSystem::SimpleRendererSystem(LveDevice &device, VkRenderPass render_pass) : lve_device_(device) {
  CreatePipelineLayout();
  CreatePipeline(render_pass);
};

SimpleRendererSystem::~SimpleRendererSystem() {
  vkDestroyPipelineLayout(lve_device_.device(), pipeline_layout_, /*alloc callback*/ nullptr);
}

void SimpleRendererSystem::CreatePipelineLayout() {
  VkPushConstantRange push_constant_range{};
  // Setting accesible from vertex and fragment stage for shared range.
  push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  // Using offset to 0 and size to sizeof struct because using it as shared range between stages
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(SimplePushConstantData);
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  // Pipeline set layout is used to pass data other than vertex data to vertex+fragment shaders.
  // for example textures, or uniform buffer objects.
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  // Push constants send a very small amount of data to shader program. Learn more in tutorial 9.
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant_range;
  if(vkCreatePipelineLayout(lve_device_.device(), &pipeline_layout_info, /*alloc callback*/ nullptr, &pipeline_layout_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout.");
  }
}

// TODO: Provide old swap chain, to optimize by reusing resources from old swapchain and better transition to full screen.
void SimpleRendererSystem::CreatePipeline(VkRenderPass render_pass) {
  // Checks that required swap chain and pipeline layout exist.
  assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");
  PipelineConfigInfo pipeline_config{};
  LvePipeline::defaultPipelineConfigInfo(pipeline_config);
  // Renderpass is like a blueprint that tells graphic pipeline what layout is
  // expected for the output frame buffer and other info.
  // Note: Now pipeline depends on this renderpass, but in the future
  // if compatible, the pipeline can work with different swap chain and render pass.
  // based on https://registry.khronos.org/vulkan/specs/1.1-extensions/html/chap8.html#renderpass-compatibility
  // TODO: Optimize by checking if render pass is compatible with pipeline, if it is, don't need to recreate pipeline.
  pipeline_config.renderPass = render_pass;
  pipeline_config.pipelineLayout = pipeline_layout_;
  lve_pipeline_ = std::make_unique<LvePipeline>(lve_device_,
                              "shaders/simple_shader.vert.spv",
                              "shaders/simple_shader.frag.spv",
                              pipeline_config);
}

void SimpleRendererSystem::RenderGameObjects(VkCommandBuffer command_buffer, std::vector<LveGameObject> &game_objects) {
  lve_pipeline_->bind(command_buffer);
  for (auto & game_obj : game_objects){
    SimplePushConstantData push_constant_data{};
    // Changing the rotation angle by 0.05 radians at every time step
    // and reset to 0 every time it reaches two_pi using mod.
    game_obj.transform2d_.rotation = glm::mod(game_obj.transform2d_.rotation + 0.01f, glm::two_pi<float>());
    push_constant_data.offset = game_obj.transform2d_.translation;
    push_constant_data.color = game_obj.color_;
    push_constant_data.transform = game_obj.transform2d_.transform();
    VkShaderStageFlags shader_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    vkCmdPushConstants(command_buffer, pipeline_layout_, shader_stages, /*offset*/ 0, /*size*/sizeof(SimplePushConstantData), &push_constant_data);
    game_obj.lve_model_->bind(command_buffer);
    game_obj.lve_model_->draw(command_buffer);
  }
}

}  // namespace lve
