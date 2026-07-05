#pragma once

#include "cam.hpp"
#include <embree4/rtcore_device.h>
#include <string>
#include <vector>

class Scene {
  RTCDevice device;
  std::vector<Vec3> vertex_buffer;
  std::vector<Vec3> normal_buffer;
  std::vector<uint32_t> index_buffer;
  std::vector<uint32_t> materials;
public:
  Camera cam;
  RTCScene scene;
  Scene(std::string const &file, uint32_t cam_idx, Camera cam);
};
