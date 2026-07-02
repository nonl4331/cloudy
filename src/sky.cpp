#include "sky.hpp"
#include <algorithm>
#include <cmath>

Sky::Sky(std::vector<Vec3> backing, uint32_t width, uint32_t height)
    : tag(Sky::Type::IMAGE), image(sky::Hdri{backing, width, height}) {}
Sky::Sky(Vec3 col) : tag(Sky::Type::SOLID), solid(col) {}

auto Sky::ray_value(Vec3 dir) -> Vec3 {
  if (tag == Sky::Type::SOLID) {
    return solid;
  }
  auto theta = dir.z / static_cast<float>(M_PI);
  auto phi = (std::atan2f(dir.y, dir.x) + static_cast<float>(M_PI)) /
             static_cast<float>(M_2_PI);
  auto x = std::clamp(phi, 0.0f, 1.0f) * static_cast<float>(image.width - 1);
  auto y = std::clamp(theta, 0.0f, 1.0f) * static_cast<float>(image.height - 1);
  return image
      .backing[static_cast<uint64_t>(x) +
               static_cast<uint64_t>(y) * static_cast<uint64_t>(image.width)];
}
