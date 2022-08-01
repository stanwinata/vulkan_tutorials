#pragma once

#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_renderer.hpp"
#include "lve_window.hpp"
#include "lve_game_object.hpp"

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
    virtual void loadGameObjects();

    LveWindow lve_window_{kWidth_, kHeight_, "Hi Vulkan!"};
    LveDevice lve_device_{lve_window_};
    std::vector<LveGameObject> lve_game_objects_;
    LveRenderer lve_renderer_{lve_window_, lve_device_};
};

}  // namespace lve