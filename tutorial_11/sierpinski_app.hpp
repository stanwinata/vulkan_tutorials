#pragma once

#include "first_app.hpp"
#include "lve_device.hpp"
#include "lve_pipeline.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"
#include "lve_model.hpp"

#include <iostream>

namespace lve {
  struct triangle {
    glm::vec2 left;
    glm::vec2 right;
    glm::vec2 top;
    int level;
  };
class SierpinskiApp : public FirstApp {
  public:
    SierpinskiApp() : FirstApp() {};
    SierpinskiApp(const SierpinskiApp &) = delete;
    SierpinskiApp &operator=(const SierpinskiApp &) = delete;

  protected:
    void generateSierpinskiVertices(std::vector<LveModel::Vertex> &vertices, int level, glm::vec2 left, glm::vec2 right, glm::vec2 top);
    // void loadModels() override;
};

}  // namespace lve