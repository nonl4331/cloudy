#include <cmath>
#include <cstdint>
#include <embree4/rtcore.h>
#include <embree4/rtcore_common.h>
#include <embree4/rtcore_ray.h>
#include <embree4/rtcore_scene.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "CLI/CLI.hpp"
#include "cam.hpp"
#include "coord.hpp"
#include "nanovdb/NanoVDB.h"
#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
using json = nlohmann::json;

#include "img.hpp"
#include "scene.hpp"
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

  std::filesystem::path config_file = input;
  std::ifstream jin(config_file);
  json data = json::parse(jin);
  std::string env_map_file = config_file.parent_path() / data["env_map"];
  std::string scene_file = config_file.parent_path() / data["scene"];

  Sky sky = load_hdri(env_map_file);
  float hfov = 60.0f * static_cast<float>(M_PI) / 180.0f;

  Scene scene(scene_file, 0,
              Camera(-vec3::Y,
                     Quaternion(std::sqrt(2) * 0.5, std::sqrt(2) * 0.5, 0, 0),
                     hfov, width, height));
  RTCIntersectArguments args;
  rtcInitIntersectArguments(&args);
  args.flags = RTC_RAY_QUERY_FLAG_COHERENT;

  std::vector<Vec3> backing(width * height, vec3::ZERO);
  for (auto x = 0; x < width; x++) {
    for (auto y = 0; y < height; y++) {
      RTCRayHit ray = scene.cam.get_stratified_ray(x + y * width);

      rtcIntersect1(scene.scene, &ray, &args);
      if (ray.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        backing[x + y * width] = sky.ray_value(ray);
      } else {
        backing[x + y * width] = vec3::ONE * std::powf(1.5f, -ray.ray.tfar);
        backing[x + y * width] =
            Vec3(ray.hit.Ng_x, ray.hit.Ng_y, ray.hit.Ng_z).normalised();
        auto id = ray.hit.primID;
        auto u = ray.hit.u;
        auto v = ray.hit.v;
        auto uv = scene.uv_buffer[scene.index_buffer[id * 3]] * (1.0f - u - v) +
                  scene.uv_buffer[scene.index_buffer[id * 3 + 1]] * u +
                  scene.uv_buffer[scene.index_buffer[id * 3 + 2]] * v;
        backing[x + y * width] = Vec3(uv.x, uv.y, 0.0f).normalised();
      }
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
