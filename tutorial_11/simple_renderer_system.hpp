#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_game_object.hpp"

#include <iostream>

namespace lve {
class SimpleRendererSystem {
  public:
    SimpleRendererSystem(LveDevice &device, VkRenderPass render_pass);
    ~SimpleRendererSystem();
    SimpleRendererSystem(const SimpleRendererSystem &) = delete;
    SimpleRendererSystem &operator=(const SimpleRendererSystem &) = delete;

    void RenderGameObjects(VkCommandBuffer command_buffer, std::vector<LveGameObject> &game_objects);
  protected:
    void CreatePipelineLayout();
    void CreatePipeline(VkRenderPass render_pass);

    LveDevice& lve_device_;
    // Using pointer for easy rather than stack allocated, makes it easy to point to new
    // swapchains. but slightly worst performance.
    std::unique_ptr<LvePipeline> lve_pipeline_;
    VkPipelineLayout pipeline_layout_;
};

}  // namespace lve