#include "lve_renderer.hpp"
#include <stdexcept>
#include <array>

namespace lve {

LveRenderer::LveRenderer(LveWindow& window, LveDevice& device) : lve_window_(window), lve_device_(device) {
  RecreateSwapChain();
  CreateCommandBuffers();
};


LveRenderer::~LveRenderer() {
  // Didn't need to free before when in FirstApp, because app and command buffer + cmd buffer pool
  // life cycle used to be tied together. but now, we can destroy renderer while app continue.
  FreeCommandBuffers();
}

void LveRenderer::CreateCommandBuffers(){
  // Advantage of command buffer is we can record once, and reuse for
  // multiple frame buffers. However since render pass which we need
  // requires target frame buffer id, we need to re-record the command buffer.
  // To simplify things, for now, we set 1 command buffer to be in charge of
  // 1 frame buffer.
  command_buffers_.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
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

void LveRenderer::RecreateSwapChain() {
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
    std::shared_ptr<LveSwapChain> old_swap_chain = std::move(lve_swap_chain_);
    lve_swap_chain_ = std::make_unique<LveSwapChain>(lve_device_, lve_window_.getExtend(), old_swap_chain);
    if (!old_swap_chain->compareSwapFormats(*lve_swap_chain_.get())) {
      throw std::runtime_error("Swap chain image(or depth) format has changed! New incompatible render pass created.");
    }
  }
}

void LveRenderer::FreeCommandBuffers() {
  vkFreeCommandBuffers(
      lve_device_.device(),
      lve_device_.getCommandPool(),
      static_cast<uint32_t>(command_buffers_.size()),
      command_buffers_.data());
  command_buffers_.clear();
}

// Starts a command frame to start recording
VkCommandBuffer LveRenderer::beginFrame() {
  assert(!is_frame_started_ && "Cannot start beginFrame when another frame is already in progess.");
  // Fetch index to the frame we should render next.
  // Automatically handle cpu-gpu synchronisation for double/triple buffering.
  // result show if it is successful.
  auto result = lve_swap_chain_->acquireNextImage(&current_img_idx_);

  // Window may gotten resized
  if(result == VK_ERROR_OUT_OF_DATE_KHR) {
    RecreateSwapChain();
    // Indicate frame not succesfully started.
    return nullptr;
  }
  if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    // Might also occur when windows is resized, we will fix this later.
    throw std::runtime_error("Failed to acquire next swapchain image");
  }
  is_frame_started_ = true;
  auto command_buffer = getCurrentCommandBuffer();
  // Record/draw command for buffer whose id is image_index.
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  // Begin recording.
  if(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer.");
  }
  return command_buffer;
}
// end the command frame recording and execute.
void LveRenderer::endFrame() {
  assert(is_frame_started_ && "Cannot get cmd_buffer when frame is not in process.");
  auto command_buffer = getCurrentCommandBuffer();
  if( vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("Failed to end recording command buffer.");
  }
  // Submit cmd_buffer to device graphic queue while handling cpu-gpu sync.
  // cmd buffer will then be executed, then swap chain will present associated
  // color attachment image view to the display at the appropriate time based on present_mode(mailbox/fifo).
  auto result = lve_swap_chain_->submitCommandBuffers(&command_buffer, &current_img_idx_);

  // Update swapchain + reset flag when window size changed.
  if (result == VK_ERROR_OUT_OF_DATE_KHR  || result == VK_SUBOPTIMAL_KHR || lve_window_.wasWindowResized()) {
    lve_window_.resetWindowResizeFlag();
    RecreateSwapChain();
  } else if(result != VK_SUCCESS) {
    throw std::runtime_error("Failed to present swap chain image.");
  }
  is_frame_started_ = false;
  current_frame_idx_ = (current_frame_idx_ + 1)% LveSwapChain::MAX_FRAMES_IN_FLIGHT;
}

// Since renderer class manage swapchain and it's render pass.
void LveRenderer::beginSwapChainRenderPass (VkCommandBuffer command_buffer) {
  assert(is_frame_started_ && "Cannot get cmd_buffer when frame is not in process.");
  assert(command_buffer == getCurrentCommandBuffer() && "Cannot start render pass on command buffer from different frame.");
  // Command to begin a render pass.
  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = lve_swap_chain_->getRenderPass();
  render_pass_info.framebuffer = lve_swap_chain_->getFrameBuffer(current_img_idx_);

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
  vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

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
  vkCmdSetViewport(command_buffer, /*first viewport*/ 0, /*viewport count*/ 1, &viewport);
  vkCmdSetScissor(command_buffer, /*first scissor*/ 0, /*scissor count*/ 1, &scissor);
}
void LveRenderer::endSwapChainRenderPass (VkCommandBuffer command_buffer) {
  assert(is_frame_started_ && "Cannot get cmd_buffer when frame is not in process.");
  assert(command_buffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from different frame.");
  vkCmdEndRenderPass(command_buffer);

}


}  // namespace lve
