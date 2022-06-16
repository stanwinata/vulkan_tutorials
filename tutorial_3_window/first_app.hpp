#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include <iostream>

namespace lve {
class FirstApp {
  public:
    void run();
    static constexpr int kWidth_ = 800;
    static constexpr int kHeight_ = 600;
  private:
    LveWindow lve_window_{kWidth_, kHeight_, "Hi Vulkan!"};
    LveDevice lve_device_{lve_window_};
    PipelineConfigInfo pipelineConfig{};
    LvePipeline lve_pipeline_{lve_device_,
                              "shaders/simple_shader.vert.spv",
                              "shaders/simple_shader.frag.spv",
                              pipelineConfig};
};

}  // namespace lve