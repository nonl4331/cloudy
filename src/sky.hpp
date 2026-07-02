#pragma once

#include "vec.hpp"
#include <cstdint>
#include <vector>

namespace sky {
struct Hdri {
  std::vector<Vec3> backing;
  uint32_t width;
  uint32_t height;
};
} // namespace sky

class Sky {
  enum struct Type {
    SOLID,
    IMAGE,
  };
  Type tag;
  union {
    Vec3 solid;
    sky::Hdri image;
  };

public:
  Sky(std::vector<Vec3> backing, uint32_t width, uint32_t height);
  Sky(Vec3 col);
  auto ray_value(Vec3 dir) -> Vec3;
};
