#include <cstdint>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <nanovdb/NanoVDB.h>
#include <spdlog/spdlog.h>

#include "vec.hpp"

auto main(int argc, char **argv) -> int {
  CLI::App app{"Volumetric Path Tracer"};
  uint32_t width = 1920;
  uint32_t height = 1080;
  uint32_t samples = 100;
  std::string out = "out.png";
  std::string input = "";
  app.add_option("-x", width, "Width")->capture_default_str();
  app.add_option("-y", height, "Height")->capture_default_str();
  app.add_option("-n", samples, "Samples")->capture_default_str();
  app.add_option("-i", input, "Input")->required(true);
  app.add_option("-o", out, "Output")->capture_default_str();

  CLI11_PARSE(app, argc, argv);

  spdlog::info(
      fmt::format("Starting render with {}x{}x{}", width, height, samples));

  std::vector<Vec3> backing(width*height, vec3::ZERO);
  for (auto x = 0; x < width; x++) {
	for (auto y = 0; y < height; y++) {
		backing[x + y*width] = Vec3((float)x / (float)(width-1), (float)y / (float)(height-1), 0.0f);
	}
  }

  int res = stbi_write_hdr("img.hdr", width, height, 3, reinterpret_cast<float*>(backing.data()));
  if (!res) {
	spdlog::error(fmt::format("stbi_write_hdr failed: {}", strerror(errno)));
  }
  return 0;
}
