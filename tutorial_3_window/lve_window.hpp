#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace lve {

class LveWindow {
  public:
    LveWindow(int w, int h, std::string window_name);
    bool ShouldClose() {return glfwWindowShouldClose(window_);}
    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
    ~LveWindow();

    // Delete copy constructor and copy operator. because it is using pointer to GLFWwindow.
    // [Theory]: Resource acquistion/creation only during initialization. and clean up performed during destructor.
    // [Intuiton]: This is to prevent copies of LveWindow, which will cause multipe instances of
    // LveWindow pointing to the same GLFWwindow pointer. Then when one of the LveWindow is deleted,
    // the others would have a dangling pointer to a non-existent GLFWwindow.
    LveWindow(const LveWindow &) = delete;
    LveWindow &operator=(const LveWindow &) = delete;

  private:
    void InitWindow();
    GLFWwindow *window_;
    int width_;
    int height_;
    std::string window_name_;

};
} // namespace lve