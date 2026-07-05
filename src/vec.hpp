#pragma once
#include <cmath>

struct Vec3 {
  float x, y, z;

  constexpr Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
  constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  constexpr static auto splat(float v) -> Vec3 { return Vec3(v, v, v); }

  constexpr auto operator==(const Vec3 &) const -> bool = default;

  constexpr auto operator+(const Vec3 &o) const -> Vec3 {
    return Vec3(x + o.x, y + o.y, z + o.z);
  }
  constexpr auto operator+=(const Vec3 &o) -> Vec3 & {
    this->x += o.x;
    this->y += o.y;
    this->z += o.z;
    return *this;
  }

  constexpr auto operator-(const Vec3 &o) const -> Vec3 {
    return Vec3(x - o.x, y - o.y, z - o.z);
  }
  constexpr auto operator-=(const Vec3 &o) -> Vec3 & {
    this->x -= o.x;
    this->y -= o.y;
    this->z -= o.z;
    return *this;
  }

  constexpr auto operator*(const float o) const -> Vec3 {
    return Vec3(x * o, y * o, z * o);
  }
  constexpr auto operator*=(const float o) -> Vec3 & {
    this->x *= o;
    this->y *= o;
    this->z *= o;
    return *this;
  }

  constexpr auto operator/(const float o) const -> Vec3 {
    const float rec = 1.0f / o;
    return Vec3(x * rec, y * rec, z * rec);
  }
  constexpr auto operator/=(const float o) -> Vec3 & {
    this->x *= (1.0f / o);
    this->y *= (1.0f / o);
    this->z *= (1.0f / o);
    return *this;
  }

  constexpr auto operator-() const -> Vec3 { return Vec3(-x, -y, -z); }

  constexpr auto dot(const Vec3 &o) const -> float {
    return x * o.x + y * o.y + z * o.z;
  }

  constexpr auto cross(const Vec3 &o) const -> Vec3 {
    return Vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
  }

  constexpr auto mag_sq() const -> float { return this->dot(*this); }

  auto mag() const -> float { return std::sqrt(this->mag_sq()); }

  auto normalised() const -> Vec3 { return *this / this->mag(); }

  auto normalise() -> void { *this = this->normalised(); }

  constexpr auto hadamard(const Vec3 &o) const -> Vec3 {
    return Vec3(x * o.x, y * o.y, z * o.z);
  }
};

namespace vec3 {
inline constexpr Vec3 ZERO = Vec3::splat(0.0f);
inline constexpr Vec3 ONE = Vec3::splat(1.0f);
inline constexpr Vec3 X = Vec3(1.0f, 0.0f, 0.0f);
inline constexpr Vec3 Y = Vec3(0.0f, 1.0f, 0.0f);
inline constexpr Vec3 Z = Vec3(0.0f, 0.0f, 1.0f);
} // namespace vec3

struct Vec2 {
  float x, y;

  constexpr Vec2() : x(0.0f), y(0.0f) {}
  constexpr Vec2(float x, float y) : x(x), y(y) {}

  constexpr static auto splat(float v) -> Vec2 { return Vec2(v, v); }

  constexpr auto operator==(const Vec2 &) const -> bool = default;

  constexpr auto operator+(const Vec2 &o) const -> Vec2 {
    return Vec2(x + o.x, y + o.y);
  }
  constexpr auto operator+=(const Vec2 &o) -> Vec2 & {
    this->x += o.x;
    this->y += o.y;
    return *this;
  }

  constexpr auto operator-(const Vec2 &o) const -> Vec2 {
    return Vec2(x - o.x, y - o.y);
  }
  constexpr auto operator-=(const Vec2 &o) -> Vec2 & {
    this->x -= o.x;
    this->y -= o.y;
    return *this;
  }

  constexpr auto operator*(const float o) const -> Vec2 {
    return Vec2(x * o, y * o);
  }
  constexpr auto operator*=(const float o) -> Vec2 & {
    this->x *= o;
    this->y *= o;
    return *this;
  }

  constexpr auto operator/(const float o) const -> Vec2 {
    const float rec = 1.0f / o;
    return Vec2(x * rec, y * rec);
  }
  constexpr auto operator/=(const float o) -> Vec2 & {
    this->x *= (1.0f / o);
    this->y *= (1.0f / o);
    return *this;
  }

  constexpr auto operator-() const -> Vec2 { return Vec2(-x, -y); }

  constexpr auto dot(const Vec2 &o) const -> float { return x * o.x + y * o.y; }

  constexpr auto mag_sq() const -> float { return this->dot(*this); }

  auto mag() const -> float { return std::sqrt(this->mag_sq()); }

  auto normalised() const -> Vec2 { return *this / this->mag(); }

  auto normalise() -> void { *this = this->normalised(); }

  constexpr auto hadamard(const Vec2 &o) const -> Vec2 {
    return Vec2(x * o.x, y * o.y);
  }
};

namespace vec2 {
inline constexpr Vec2 ZERO = Vec2::splat(0.0f);
inline constexpr Vec2 ONE = Vec2::splat(1.0f);
inline constexpr Vec2 X = Vec2(1.0f, 0.0f);
inline constexpr Vec2 Y = Vec2(0.0f, 1.0f);
} // namespace vec2
