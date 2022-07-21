#include "first_app.hpp"
#include "sierpinski_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
  /* Uncomment below to run vanilla first_app */
  lve::FirstApp app;
  /* Uncomment above to run vanilla first_app */

  /* Uncomment below to run Sierpinski Triangle */
  // Using Template Method Pattern in C++ S.T we can call overriding function in init()
  // lve::SierpinskiApp sierpinski_app;
  // lve::FirstApp &app = sierpinski_app;
  /* Uncomment above to run Sierpinski Triangle */

  try {
    app.init();
    app.run();
  } catch(const std::exception &e) {
    std::cerr << e.what() <<"\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}