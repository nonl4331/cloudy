#include "cam.hpp"
#include "coord.hpp"
#include <cmath>
#include <embree4/rtcore_common.h>
#include <limits>
#include <spdlog/spdlog.h>

// see
// https://math.stackexchange.com/questions/40164/how-do-you-rotate-a-vector-by-a-unit-quaternion
// and https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
// match blender default where default is up = Y, forward = -Z
// code is from my other PT: https://github.com/nonl4331/yapt.git
Camera::Camera(Vec3 origin, Quaternion rot, float hfov_rad, uint32_t width,
               uint32_t height)
    : origin(origin), width(width), height(height) {
  this->up =
      rot.hamilton(Quaternion(Vec3(0.0, 1.0, 0.0))).hamilton(rot.conj()).xyz();

  auto forward =
      rot.hamilton(Quaternion(Vec3(0.0, 0.0, -1.0))).hamilton(rot.conj()).xyz();
  auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

  auto right_mag = 2.0 * std::tan(0.5 * hfov_rad);
  auto up_mag = right_mag / aspect_ratio;

  auto right = forward.cross(this->up) * right_mag;
  this->up *= up_mag;

  this->lower_left = origin - right * 0.5f - this->up * 0.5f + forward;
  this->right = right;
}

auto Camera::get_stratified_ray(uint64_t i) -> RTCRayHit {
  RTCRayHit rayhit;

  rayhit.ray.org_x = this->origin.x;
  rayhit.ray.org_y = this->origin.y;
  rayhit.ray.org_z = this->origin.z;
  rayhit.ray.tnear = 0.0f;
  rayhit.ray.tfar = std::numeric_limits<float>::infinity();
  rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

  float u = (static_cast<float>(i % static_cast<uint64_t>(width)) + 0.5f) /
            static_cast<float>(width);
  float v = (static_cast<float>(i / static_cast<uint64_t>(width)) + 0.5f) /
            static_cast<float>(height);

  Vec3 dir = (lower_left + right * u + up * (1.0f - v) - origin).normalised();

  rayhit.ray.dir_x = dir.x;
  rayhit.ray.dir_y = dir.y;
  rayhit.ray.dir_z = dir.z;

  return rayhit;
}
