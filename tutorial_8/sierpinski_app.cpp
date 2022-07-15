#include "sierpinski_app.hpp"
#include "sierpinski_app.hpp"
#include <stdexcept>
#include <array>
#include <deque>

namespace lve {
  void SierpinskiApp::generateSierpinskiVertices(std::vector<LveModel::Vertex> &vertices,
                                            int end_level, glm::vec2 left, glm::vec2 right,
                                            glm::vec2 top) {
    int top_level = 0;
    std::deque<triangle> candidates{{left, right, top, top_level}};
    while(!candidates.empty()) {
      triangle node_to_explore = candidates.front();
      candidates.pop_front();
      if(node_to_explore.level == end_level) {
        vertices.push_back({node_to_explore.top, {1.0f, 0.0f, 0.0f}});
        vertices.push_back({node_to_explore.left, {0.0f, 1.0f, 0.0f}});
        vertices.push_back({node_to_explore.right, {0.0f, 0.0f, 1.0f}});
      } else {
        int new_level = node_to_explore.level + 1;
        glm::vec2 mid_leftop = 0.5f * (node_to_explore.left + node_to_explore.top);
        glm::vec2 mid_rightop = 0.5f * (node_to_explore.right + node_to_explore.top);
        glm::vec2 mid_leftright = 0.5f * (node_to_explore.left + node_to_explore.right);
        candidates.push_back({mid_leftop, mid_rightop, node_to_explore.top, new_level});
        candidates.push_back({node_to_explore.left, mid_leftright, mid_leftop, new_level});
        candidates.push_back({mid_leftright, node_to_explore.right, mid_rightop, new_level});
      }
    }
    return;
  }

  void SierpinskiApp::loadModels() {
    std::vector<LveModel::Vertex> vertices{};
    int level = 5;
    generateSierpinskiVertices(vertices, level, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
    lve_model_ = std::make_unique<LveModel>(lve_device_, vertices);
  }

} // namespace lve