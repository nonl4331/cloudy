#pragma once

#include "vec.hpp"
#include <cstdint>
#include <embree4/rtcore_ray.h>
#include <utility>
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
  Sky();
  Sky(std::vector<Vec3> backing, uint32_t width, uint32_t height);
  Sky(Vec3 col);
  auto ray_value(RTCRayHit ray) -> Vec3;
  // rule of 5
  ~Sky() {
	if (tag == Type::IMAGE) {
		image.~Hdri();
	}
  }
  // copy
  Sky(const Sky& o): tag(o.tag) {
	if (tag == Type::IMAGE) {
		image = o.image;
	} else {
		solid = o.solid;
	}
  }
  // move
  Sky(Sky&& o): tag(o.tag) {
	if (tag == Type::IMAGE) {
		image = o.image;
	} else {
		solid = o.solid;
	}
	o = Sky();
  }
  // copy assignment
  auto operator=(const Sky& o) -> Sky& {
	return *this = Sky(o);
  }
  // move assignment
  auto operator=(Sky&& o) -> Sky& {
	*this = Sky(o);
	return *this;
  }
};
