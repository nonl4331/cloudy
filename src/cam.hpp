#pragma once

#include "coord.hpp"
#include "vec.hpp"
#include <cstdint>
#include <embree4/rtcore.h>

class Camera {
public:
  Vec3 lower_left;
  Vec3 up;
  Vec3 right;
  Vec3 origin;
  uint32_t width;
  uint32_t height;

  Camera(Vec3 origin, Quaternion rot, float vfov_rad, uint32_t width,
         uint32_t height);

  auto get_stratified_ray(uint64_t i) -> RTCRayHit;
  auto get_aspect_ratio() -> float;
  auto get_width() -> uint32_t;
  auto get_height() -> uint32_t;
};
