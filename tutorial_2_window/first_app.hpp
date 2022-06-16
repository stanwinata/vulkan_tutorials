#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"

namespace lve {
class FirstApp {
  public:
    void run();
    static constexpr int kWidth_ = 800;
    static constexpr int kHeight_ = 600;
  private:
    LveWindow lve_window_{kWidth_, kHeight_, "Hi Vulkan!"};
    LvePipeline lve_pipeline_{"shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv"};
};

}  // namespace lve