#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"
#include "lve_model.hpp"

#include <iostream>

namespace lve {
class FirstApp {
  public:
    void run();
    void init();
    static constexpr int kWidth_ = 800;
    static constexpr int kHeight_ = 600;
    FirstApp();
    ~FirstApp();
    FirstApp(const FirstApp &) = delete;
    FirstApp &operator=(const FirstApp &) = delete;

  protected:
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandBuffers();
    void drawFrame();
    void bind(VkCommandBuffer command_buffer);
    virtual void loadModels();

    LveWindow lve_window_{kWidth_, kHeight_, "Hi Vulkan!"};
    LveDevice lve_device_{lve_window_};
    LveSwapChain lve_swap_chain_{lve_device_, lve_window_.getExtend()};
    // PipelineConfigInfo pipelineConfig{};
    std::unique_ptr<LvePipeline> lve_pipeline_;
    VkPipelineLayout pipeline_layout_;
    std::vector<VkCommandBuffer> command_buffers_;
    std::unique_ptr<LveModel> lve_model_;
};

}  // namespace lve