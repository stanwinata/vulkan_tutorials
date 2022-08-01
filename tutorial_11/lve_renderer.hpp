#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

#include <iostream>

namespace lve {
class LveRenderer {
  public:
    LveRenderer(LveWindow &window, LveDevice &device);
    ~LveRenderer();
    LveRenderer(const LveRenderer &) = delete;
    LveRenderer &operator=(const LveRenderer &) = delete;

    // Starts a command frame to start recording
    VkCommandBuffer beginFrame();
    // end the command frame recording and execute.
    void endFrame();

    // Since renderer class manage swapchain and it's render pass.
    void beginSwapChainRenderPass (VkCommandBuffer command_buffer);
    void endSwapChainRenderPass (VkCommandBuffer command_buffer);

    // Tracking state of current in-progress frame.
    VkRenderPass getSwapChainRenderPass() const {
      return lve_swap_chain_->getRenderPass();
    }

    bool isFrameInProgess() const {
      return is_frame_started_;
    }

    int getFrameIndex() {
      assert(is_frame_started_ && "Cannot get cmd_buffer when frame is not in process");
      return current_frame_idx_;
    }

    VkCommandBuffer getCurrentCommandBuffer () const {
      assert(is_frame_started_ && "Cannot get cmd_buffer when frame is not in process");
      return command_buffers_[current_frame_idx_];
    }


  protected:
    void CreateCommandBuffers();
    void drawFrame();
    void RecreateSwapChain();
    void FreeCommandBuffers();

    LveWindow& lve_window_;
    LveDevice& lve_device_;
    // Using pointer for easy rather than stack allocated, makes it easy to point to new
    // swapchains. but slightly worst performance.
    std::unique_ptr<LveSwapChain> lve_swap_chain_;
    std::vector<VkCommandBuffer> command_buffers_;

    uint32_t current_img_idx_{0};
    // Track frames (this makes frame independent from image.)
    int current_frame_idx_{0};
    bool is_frame_started_{false};
};

}  // namespace lve