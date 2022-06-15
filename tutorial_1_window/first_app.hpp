#pragma once

#include "lve_window.hpp"

namespace lve {
class FirstApp {
  public:
    void run();
    static constexpr int kWidth_ = 800;
    static constexpr int kHeight_ = 600;
  private:
    LveWindow lve_window_{kWidth_, kHeight_, "Hi Vulkan!"};
};

}  // namespace lve