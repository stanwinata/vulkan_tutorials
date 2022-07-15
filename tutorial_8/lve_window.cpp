#include "lve_window.hpp"

namespace lve {

LveWindow::LveWindow(int w, int h, std::string window_name) : width_{w}, height_{h}, window_name_{window_name} {
  InitWindow();
}

LveWindow::~LveWindow() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void LveWindow::InitWindow() {
  glfwInit();
  // GLFW Used to create opengl context, since we don't use it we use the GLFW_NO_API to fix this.
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  // 4th param - for full screen, but dont want that so nulltpr.
  // 5th param - for opengl context, but dont have that so nulltpr.
  window_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
  // Pairs GLFWwindow object to a pointer, which in this case is the parent LveWindow object.
  glfwSetWindowUserPointer(window_, this);
  // Point callback of resizing window_ to our frameBufferResizedCallback.
  glfwSetFramebufferSizeCallback(window_, frameBufferResizedCallback);
}

void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if(glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Window surface");
  }
}

void LveWindow::frameBufferResizedCallback(GLFWwindow *window, int width, int height) {
  auto lve_window = reinterpret_cast<LveWindow *>(window);
  lve_window->frame_buffer_resized_ = true;
  lve_window->width_ = width;
  lve_window->height_ = height;
}


}  // namespace lve
