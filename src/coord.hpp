#pragma once

#include "vec.hpp"

class Quaternion {
  float w;
  float x;
  float y;
  float z;

public:
  Quaternion(float w, float x, float y, float z);
  Quaternion(Vec3 vec);
  auto hamilton(const Quaternion &o) const -> Quaternion;
  auto xyz() -> Vec3;
  auto conj() -> Quaternion;
};
