#include <cstdint>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "nanovdb/NanoVDB.h"
#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "img.hpp"
#include "vec.hpp"

auto main(int argc, char **argv) -> int {
  CLI::App app{"Volumetric Path Tracer"};
  uint32_t width = 1920;
  uint32_t height = 1080;
  uint32_t samples = 100;
  std::string out = "out";
  std::string input = "";
  char output_mode = 'b';
  app.add_option("-x", width, "Width")
      ->capture_default_str()
      ->check(CLI::Range((uint32_t)1, (uint32_t)4294967295));
  app.add_option("-y", height, "Height")
      ->capture_default_str()
      ->check(CLI::Range((uint32_t)1, (uint32_t)4294967295));
  app.add_option("-n", samples, "Samples")
      ->capture_default_str()
      ->check(CLI::Range((uint32_t)1, (uint32_t)4294967295));
  app.add_option("-i", input, "Input")->required(true);
  app.add_option("-o", out, "Output Name (without extension)")
      ->capture_default_str();
  app.add_option(
         "-m", output_mode,
         "Output Mode (h: hdr only, s: sdr only, n: no output, b: both)")
      ->capture_default_str()
      ->check(CLI::Validator(
          [](std::string &c) {
            if (c != "h" && c != "s" && c != "n" && c != "b") {
              throw CLI::ValidationError(
                  fmt::format("Invalid output mode {}", c));
            }
            return "";
          },
          "CHAR in {h, s, n, b}", "Output Mode Validator"));

  CLI11_PARSE(app, argc, argv);

  spdlog::info("Starting render with {}x{}x{}", width, height, samples);

  std::vector<Vec3> backing(width * height, vec3::ZERO);
  for (auto x = 0; x < width; x++) {
    for (auto y = 0; y < height; y++) {
      backing[x + y * width] = Vec3((float)x / (float)(width - 1),
                                    (float)y / (float)(height - 1), 0.0f) *
                               10.0f;
    }
  }

  if (output_mode == 's' || output_mode == 'b') {
    write_sdr_image(backing, width, height, out);
  }
  if (output_mode == 'h' || output_mode == 'b') {
    write_hdr_image(backing, width, height, out);
  }

  return 0;
}
