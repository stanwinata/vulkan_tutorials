#include "first_app.hpp"
#include <stdexcept>
#include <array>

namespace lve {

FirstApp::FirstApp() {};

void FirstApp::init() {
  loadModels();
  CreatePipelineLayout();
  RecreateSwapChain();
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

// TODO: Provide old swap chain, to optimize by reusing resources from old swapchain and better transition to full screen.
void FirstApp::CreatePipeline() {
  // Checks that required swap chain and pipeline layout exist.
  assert(lve_swap_chain_ != nullptr && "Cannot create pipeline before swap chain");
  assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");
  PipelineConfigInfo pipeline_config{};
  LvePipeline::defaultPipelineConfigInfo(pipeline_config);
  // Renderpass is like a blueprint that tells graphic pipeline what layout is
  // expected for the output frame buffer and other info.
  // Note: Now pipeline depends on this renderpass, but in the future
  // if compatible, the pipeline can work with different swap chain and render pass.
  // based on https://registry.khronos.org/vulkan/specs/1.1-extensions/html/chap8.html#renderpass-compatibility
  // TODO: Optimize by checking if render pass is compatible with pipeline, if it is, don't need to recreate pipeline.
  pipeline_config.renderPass = lve_swap_chain_->getRenderPass();
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
  command_buffers_.resize(lve_swap_chain_->imageCount());
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
};

void FirstApp::RecordCommandBuffer(int image_index) {
  // Record/draw command for buffer whose id is image_index.
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // Begin recording.
  if(vkBeginCommandBuffer(command_buffers_[image_index], &begin_info) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer");
  }
  // Command to begin a render pass.
  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = lve_swap_chain_->getRenderPass();
  render_pass_info.framebuffer = lve_swap_chain_->getFrameBuffer(image_index);

  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = lve_swap_chain_->getSwapChainExtent();

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
  vkCmdBeginRenderPass(command_buffers_[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

  // Viewport describe transformation between pipeline output and target image.
  // pipeline output is from (-1,-1) to (1,1) where (0,0) is center.
  // Viewport convert from pipeline output to pixel coordinates.
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(lve_swap_chain_->getSwapChainExtent().width);
  viewport.height = static_cast<float>(lve_swap_chain_->getSwapChainExtent().height);
  // Depth range for viewport to linear transform z position.
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Scissor cuts images, any pixel outside of the sciccor rectangle
  // described below will get discarded.
  // offset = {0, 0} + extent = swapchain's extent.
  VkRect2D scissor{{0, 0}, lve_swap_chain_->getSwapChainExtent()};

  // vkCmdSetViewport and vkCmdSetScissor can only be invoked iff pipeline's dynamic state is configured.
  vkCmdSetViewport(command_buffers_[image_index], /*first viewport*/ 0, /*viewport count*/ 1, &viewport);
  vkCmdSetScissor(command_buffers_[image_index], /*first scissor*/ 0, /*scissor count*/ 1, &scissor);

  lve_pipeline_->bind(command_buffers_[image_index]);
  lve_model_->bind(command_buffers_[image_index]);
  lve_model_->draw(command_buffers_[image_index]);
  vkCmdEndRenderPass(command_buffers_[image_index]);
  if( vkEndCommandBuffer(command_buffers_[image_index]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to end recording command buffer");
  }
}

void FirstApp::RecreateSwapChain() {
  auto extent = lve_window_.getExtend();
  // If window is minimized.
  while(extent.width == 0 || extent.height == 0) {
    extent =lve_window_.getExtend();
    // Puts thread to sleep until new events detected / Wait for new events
    glfwWaitEvents();
  }
  // To wait on the host for the completion of outstanding queue operations for all queues on a given logical device.
  vkDeviceWaitIdle(lve_device_.device());

  // If there exist no current swapchain make a fresh one.
  // Otherwise make a new one that is based on old one for optimizing by reuse of resources.
  if(lve_swap_chain_ == nullptr) {
    lve_swap_chain_ = std::make_unique<LveSwapChain>(lve_device_, lve_window_.getExtend());
  } else {
    // Using move, since lve_swap_chain_ is unique pointer, s.t we can move the resource to the shared_ptr that
    // we will be using for the new swap chain constructor.
    lve_swap_chain_ = std::make_unique<LveSwapChain>(lve_device_, lve_window_.getExtend(), std::move(lve_swap_chain_));
    // If frame buffer count is not same with current command buffer count anymore.
    // Re-initialize and re-record comman buffers,
    if(lve_swap_chain_->imageCount() != command_buffers_.size()) {
      FreeCommandBuffers();
      CreateCommandBuffers();
    }
  }
  // need to re invoke createPipeline, to update data with new swapchain data.
  CreatePipeline();
}
void FirstApp::drawFrame(){
  uint32_t image_index;
  // Fetch index to the frame we should render next.
  // Automatically handle cpu-gpu synchronisation for double/triple buffering.
  // result show if it is successful.
  auto result = lve_swap_chain_->acquireNextImage(&image_index);

  // Window may gotten resized
  if(result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
    return;
  }
  if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    // Might also occur when windows is resized, we will fix this later.
    throw std::runtime_error("Failed to acquire next swapchain image");
  }
  // Record command buffer before submitting.
  RecordCommandBuffer(image_index);
  // Submit cmd_buffer to device graphic queue while handling cpu-gpu sync.
  // cmd buffer will then be executed, then swap chain will present associated
  // color attachment image view to the display at the appropriate time based on present_mode(mailbox/fifo).
  result = lve_swap_chain_->submitCommandBuffers(&command_buffers_[image_index], &image_index);

  // Update swapchain + reset flag when window size changed.
  if (result == VK_ERROR_OUT_OF_DATE_KHR  || result == VK_SUBOPTIMAL_KHR || lve_window_.wasWindowResized()) {
    RecreateSwapChain();
    lve_window_.resetWindowResizeFlag();
    return;
  }
  if(result != VK_SUCCESS) {
    throw std::runtime_error("Failed to present swap chain image.");
  }
};

void FirstApp::FreeCommandBuffers() {
  vkFreeCommandBuffers(
      lve_device_.device(),
      lve_device_.getCommandPool(),
      static_cast<uint32_t>(command_buffers_.size()),
      command_buffers_.data());
  command_buffers_.clear();
}
}  // namespace lve
