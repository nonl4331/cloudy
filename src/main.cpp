#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lib.hpp"

auto main() -> int {
  auto const lib = library{};
  auto const message = "Hello from " + lib.name + "!";
  spdlog::info(message);
  return 0;
}
