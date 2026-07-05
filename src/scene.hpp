#pragma once

#include "cam.hpp"
#include <embree4/rtcore_device.h>
#include <string>
#include <vector>

class Scene {
public:
  RTCDevice device;
  std::vector<Vec3> vertex_buffer;
  std::vector<Vec3> normal_buffer;
  std::vector<uint32_t> index_buffer;
  std::vector<uint32_t> materials;
  std::vector<Vec2> uv_buffer;
  Camera cam;
  RTCScene scene;
  Scene(std::string const &file, uint32_t cam_idx, Camera cam);
};
