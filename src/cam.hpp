#pragma once

#include "coord.hpp"
#include "vec.hpp"
#include <cstdint>
#include <embree4/rtcore.h>

class Camera {
  Vec3 lower_left;
  Vec3 up;
  Vec3 right;
  Vec3 origin;
  uint32_t width;
  uint32_t height;

public:
  Camera(Vec3 origin, Quaternion rot, float hfov_rad, uint32_t width,
         uint32_t height);

  auto get_stratified_ray(uint64_t i) -> RTCRayHit;
};
