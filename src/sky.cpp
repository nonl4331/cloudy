#include "sky.hpp"
#include <algorithm>
#include <cmath>
#include <spdlog/spdlog.h>

Sky::Sky() : tag(Sky::Type::SOLID), solid(vec3::ZERO) {}
Sky::Sky(std::vector<Vec3> backing, uint32_t width, uint32_t height)
    : tag(Sky::Type::IMAGE), image(sky::Hdri{backing, width, height}) {}
Sky::Sky(Vec3 col) : tag(Sky::Type::SOLID), solid(col) {}

auto Sky::ray_value(RTCRayHit ray) -> Vec3 {
  if (tag == Sky::Type::SOLID) {
    return solid;
  }
  auto theta = std::acos(ray.ray.dir_z) / static_cast<float>(M_PI);
  auto phi = std::atan2f(ray.ray.dir_y, ray.ray.dir_x);
  if (phi < 0.0f) {
    phi += static_cast<float>(2 * M_PI);
  }
  phi /= static_cast<float>(2 * M_PI);

  auto x = std::clamp(phi, 0.0f, 1.0f) * static_cast<float>(image.width - 1);
  auto y = std::clamp(theta, 0.0f, 1.0f) * static_cast<float>(image.height - 1);

  return image
      .backing[static_cast<uint64_t>(x) +
               static_cast<uint64_t>(y) * static_cast<uint64_t>(image.width)];
}
