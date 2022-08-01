#include "first_app.hpp"
#include "simple_renderer_system.hpp"
#include <stdexcept>
#include <array>
#include <glm/gtc/constants.hpp>

namespace lve {

FirstApp::FirstApp() {};

void FirstApp::init() {
  loadGameObjects();
}

FirstApp::~FirstApp() {}
void FirstApp::run() {
  SimpleRendererSystem simple_render_system{lve_device_, lve_renderer_.getSwapChainRenderPass()};
  while (!lve_window_.ShouldClose()) {
    glfwPollEvents();
    if(auto command_buffer = lve_renderer_.beginFrame()) {
      // Splitting beginSwapChainRenderPass and beginFrame, in order
      // to enable multiple render passes in the future(e.g reflection, shadow, postprocess).
      lve_renderer_.beginSwapChainRenderPass(command_buffer);
      simple_render_system.RenderGameObjects(command_buffer, lve_game_objects_);
      lve_renderer_.endSwapChainRenderPass(command_buffer);
      lve_renderer_.endFrame();
    }
  }
  // cpu block all operation until gpu operation complete.
  // To prevent error when closing window which might be at same time
  // as commmand buffer execution causing destructor to get called
  // while resources are used.
  vkDeviceWaitIdle(lve_device_.device());
}

void FirstApp::loadGameObjects() {
  // Initialize a vector of Vertex, but only give input to vec2/position.
  // to the position.
  std::vector<LveModel::Vertex> vertices {
    {{0.0, -0.5}, {1.0f, 0.0f, 0.0f}},
    {{0.5, 0.5}, {0.f, 1.0f, 0.0f}},
    {{-0.5, 0.5}, {0.0f, 0.0f, 1.0f}}
  };
  // Shared so that multiple game objects can use the same model.
  auto lve_model = std::make_shared<LveModel>(lve_device_, vertices);
  LveGameObject triangle = LveGameObject::createGameObject();
  triangle.lve_model_ = lve_model;
  triangle.color_ = {0.1f, 0.8f, 0.1f};
  triangle.transform2d_.translation.x = 0.2f;
  triangle.transform2d_.scale = {2.0f, 0.5f};
  triangle.transform2d_.rotation = 0.25f * glm::two_pi<float>();
  lve_game_objects_.push_back(std::move(triangle));
}

}  // namespace lve
