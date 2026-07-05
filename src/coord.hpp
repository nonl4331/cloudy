#pragma once

#include "vec.hpp"

struct Quaternion {
  float w;
  float x;
  float y;
  float z;
  constexpr Quaternion(float w, float x, float y, float z)
      : w(w), x(x), y(y), z(z) {};
  constexpr Quaternion(Vec3 vec) : w(0.0f), x(vec.x), y(vec.y), z(vec.z) {}
  constexpr auto hamilton(const Quaternion &o) const -> Quaternion {
    return Quaternion(w * o.w - x * o.x - y * o.y - z * o.z,
                      w * o.x + x * o.w + y * o.z - z * o.y,
                      w * o.y - x * o.z + y * o.w + z * o.x,
                      w * o.z + x * o.y - y * o.x + z * o.w);
  }
  constexpr auto xyz() -> Vec3 { return Vec3(x, y, z); }
  constexpr auto conj() -> Quaternion { return Quaternion(w, -x, -y, -z); }
};

namespace quaternion {
inline constexpr Quaternion IDENTITY = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
}
