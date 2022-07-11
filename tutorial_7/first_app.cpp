#include "first_app.hpp"
#include <stdexcept>
#include <array>

namespace lve {

FirstApp::FirstApp() {};

void FirstApp::init() {
  loadModels();
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
    drawFrame();
  }
  // cpu block all operation until gpu operation complete.
  // To prevent error when closing window which might be at same time
  // as commmand buffer execution causing destructor to get called
  // while resources are used.
  vkDeviceWaitIdle(lve_device_.device());
}

void FirstApp::loadModels() {
  // Initialize a vector of Vertex, but only give input to vec2/position.
  // to the position.
  std::cout<<"base class!\n";
  std::vector<LveModel::Vertex> vertices {
    {{0.0, -0.5}, {1.0f, 0.0f, 0.0f}},
    {{0.5, 0.5}, {0.f, 1.0f, 0.0f}},
    {{-0.5, 0.5}, {0.0f, 0.0f, 1.0f}}
  };
  lve_model_ = std::make_unique<LveModel>(lve_device_, vertices);
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

void FirstApp::CreateCommandBuffers(){
  // Advantage of command buffer is we can record once, and reuse for
  // multiple frame buffers. However since render pass which we need
  // requires target frame buffer id, we need to re-record the command buffer.
  // To simplify things, for now, we set 1 command buffer to be in charge of
  // 1 frame buffer.
  command_buffers_.resize(lve_swap_chain_.imageCount());
  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  // Primary = can be sent to queue for execution. but cannot be called by other command buffers.
  // Secondary = cannot be sent to queue for execution. but can be called by other command buffers.
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  // Command pools = opaque objects that command buffer memory is allocated from.
  // Application need to create and destroy command buffer frequently,
  // so it takes it from command pool to reduce cost of resource creation.
  // (Somewhat of a caching allocator or special memory regions reserved for cmd buffers)
  alloc_info.commandPool = lve_device_.getCommandPool();
  alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());
  if(vkAllocateCommandBuffers(lve_device_.device(), &alloc_info, command_buffers_.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command buffer.");
  }
  // Record/draw command for each buffer.
  for (int i = 0; i < command_buffers_.size(); i++) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // Begin recording.
    if(vkBeginCommandBuffer(command_buffers_[i], &begin_info) != VK_SUCCESS) {
      throw std::runtime_error("Failed to begin recording command buffer");
    }
    // Command to begin a render pass.
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = lve_swap_chain_.getRenderPass();
    render_pass_info.framebuffer = lve_swap_chain_.getFrameBuffer(i);

    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = lve_swap_chain_.getSwapChainExtent();

    // Set clear value to initialize value of frame buffer's attachments/data.
    // Using 2, because one for color attachments and another for depth.
    std::array<VkClearValue, 2> clear_values{};
    // Set clear_value[0] as our color attachment to the current frame buffer. Where defined by RGBA.
    clear_values[0].color = {0.1f, 0.1f, 0.1f, 0.1f};
    // Set clear_value[1] as our depth attachment to the current frame buffer. where farthest value is 1 and closest is 0.
    clear_values[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount = clear_values.size();
    render_pass_info.pClearValues = clear_values.data();
    //VK_SUBPASS_CONTENT_INLINE = The commands following this line will be directly embedded to primary command buffer and
    // no secondary command buffer will be used.
    //Alternatively, VK_SUBPASS_SECONDARY_COMMAND_BUFFERS = Then the following render pass commands will be using secondary command
    // buffers. and no primary command buffers will be used.
    // This implies no mixing allowed in render pass to use both primary and secondary command buffers.
    vkCmdBeginRenderPass(command_buffers_[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    lve_pipeline_->bind(command_buffers_[i]);
    lve_model_->bind(command_buffers_[i]);
    lve_model_->draw(command_buffers_[i]);
    vkCmdEndRenderPass(command_buffers_[i]);
    if( vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to end recording command buffer");
    }
  }
};
void FirstApp::drawFrame(){
  uint32_t image_index;
  // Fetch index to the frame we should render next.
  // Automatically handle cpu-gpu synchronisation for double/triple buffering.
  // result show if it is successful.
  auto result = lve_swap_chain_.acquireNextImage(&image_index);
  if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    // Might also occur when windows is resized, we will fix this later.
    throw std::runtime_error("Failed to acquire next swapchain image");
  }
  // Submit cmd_buffer to device graphic queue while handling cpu-gpu sync.
  // cmd buffer will then be executed, then swap chain will present associated
  // color attachment image view to the display at the appropriate time based on present_mode(mailbox/fifo).
  result = lve_swap_chain_.submitCommandBuffers(&command_buffers_[image_index], &image_index);
  if(result != VK_SUCCESS) {
    throw std::runtime_error("Failed to present swap chain image.");
  }
};

}  // namespace lve
