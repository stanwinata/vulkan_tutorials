#include "first_app.hpp"
#include <stdexcept>


namespace lve {

FirstApp::FirstApp() {
  CreatePipelineLayout();
  CreatePipeline();
  CreateCommandBuffers();
}

FirstApp::~FirstApp() {
  vkDestroyPipelineLayout(lve_device_.device(), pipeline_layout_, /*alloc callback*/ nullptr);
}
void FirstApp::run() {
  while (!lve_window_.ShouldClose()) {
    glfwPollEvents();
  }
}

void FirstApp::CreatePipelineLayout() {
  VkPipelineLayoutCreateInfo pipeline_layout_info{};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  // Pipeline set layout is used to pass data other than vertex data to vertex+fragment shaders.
  // for example textures, or uniform buffer objects.
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  // Push constants send a very small amount of data to shader program. Learn more in tutorial 9.
  pipeline_layout_info.pushConstantRangeCount = 0;
  pipeline_layout_info.pPushConstantRanges = nullptr;
  if(vkCreatePipelineLayout(lve_device_.device(), &pipeline_layout_info, /*alloc callback*/ nullptr, &pipeline_layout_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout.");
  }
}

void FirstApp::CreatePipeline() {
  auto pipeline_config = LvePipeline::defaultPipelineConfigInfo(lve_swap_chain_.width(), lve_swap_chain_.height());
  pipeline_config.renderPass = lve_swap_chain_.getRenderPass();
  pipeline_config.pipelineLayout = pipeline_layout_;
  lve_pipeline_ = std::make_unique<LvePipeline>(lve_device_,
                              "shaders/simple_shader.vert.spv",
                              "shaders/simple_shader.frag.spv",
                              pipeline_config);
}

void FirstApp::CreateCommandBuffers(){};
void FirstApp::drawFrame(){};

}  // namespace lve
