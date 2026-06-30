#pragma once

#include "vec.hpp"
#include <cstdint>
#include <string>
#include <vector>

auto write_hdr_image(std::vector<Vec3> const &img, uint32_t width,
                     uint32_t height, std::string const &name) -> void;

auto write_sdr_image(std::vector<Vec3> const &img, uint32_t width,
                     uint32_t height, std::string const &name) -> void;
