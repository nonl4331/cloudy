#pragma once
#include <cmath>

struct Vec3 {
  float x, y, z;

  constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  constexpr static auto splat(float v) -> Vec3 { return Vec3(v, v, v); }

  auto operator+(const Vec3 &o) const -> Vec3 {
    return Vec3(x + o.x, y + o.y, z + o.z);
  }
  auto operator+=(const Vec3 &o) -> Vec3 { return *this = *this + o; }

  auto operator-(const Vec3 &o) const -> Vec3 {
    return Vec3(x - o.x, y - o.y, z - o.z);
  }
  auto operator-=(const Vec3 &o) -> Vec3 { return *this = *this - o; }

  auto operator*(const float o) const -> Vec3 {
    return Vec3(x * o, y * o, z * o);
  }
  auto operator*=(const float o) -> Vec3 { return *this = *this * o; }

  auto operator/(const float o) const -> Vec3 {
    return Vec3(x, y, z) * (1 / o);
  }
  auto operator/=(const float o) -> Vec3 { return *this = *this / o; }

  auto operator-() const -> Vec3 { return Vec3(-x, -y, -z); }

  auto dot(const Vec3 &o) const -> float { return x * o.x + y * o.y + z * o.z; }

  auto mag_sq() const -> float { return this->dot(*this); }

  auto mag() const -> float { return sqrt(this->mag_sq()); }

  auto normalised() const -> Vec3 { return *this / this->mag(); }

  auto normalise() -> void { *this = this->normalised(); }
};

namespace vec3 {
inline constexpr Vec3 ZERO = Vec3::splat(0.0f);
inline constexpr Vec3 ONE = Vec3::splat(1.0f);
inline constexpr Vec3 X = Vec3(1.0f, 0.0f, 0.0f);
inline constexpr Vec3 Y = Vec3(0.0f, 1.0f, 0.0f);
inline constexpr Vec3 z = Vec3(0.0f, 0.0f, 1.0f);
} // namespace vec3
