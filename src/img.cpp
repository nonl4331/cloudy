#include <algorithm>
#include <math.h>

#include "img.hpp"
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfOutputFile.h>
#include <ImfStandardAttributes.h>

using namespace OPENEXR_IMF_NAMESPACE;

// for now use rec709 + 80nit D65 white
auto write_hdr_image(std::vector<Vec3> const &img, uint32_t width,
                     uint32_t height, std::string const &name) -> void {
  std::vector<float> r(width * height);
  std::vector<float> g(width * height);
  std::vector<float> b(width * height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint32_t px_idx = x + y * width;
      r[px_idx] = img[px_idx].x;
      g[px_idx] = img[px_idx].y;
      b[px_idx] = img[px_idx].z;
    }
  }

  try {
    Imf::Chromaticities rec709(
        Imath::V2f(0.6400f, 0.3300f), Imath::V2f(0.3000f, 0.6000f),
        Imath::V2f(0.1500f, 0.0600f), Imath::V2f(0.3127f, 0.3290f));

    Imf::Header header(width, height);
    Imf::addChromaticities(header, rec709);
    Imf::addWhiteLuminance(header, 80.0f);

    header.channels().insert("R", Imf::Channel(Imf::FLOAT));
    header.channels().insert("G", Imf::Channel(Imf::FLOAT));
    header.channels().insert("B", Imf::Channel(Imf::FLOAT));

    Imf::OutputFile file(fmt::format("{}.exr", name).c_str(), header);

    Imf::FrameBuffer fb;
    fb.insert("R", Imf::Slice(Imf::FLOAT, reinterpret_cast<char *>(r.data()),
                              sizeof(float), sizeof(float) * width));
    fb.insert("G", Imf::Slice(Imf::FLOAT, reinterpret_cast<char *>(g.data()),
                              sizeof(float), sizeof(float) * width));
    fb.insert("B", Imf::Slice(Imf::FLOAT, reinterpret_cast<char *>(b.data()),
                              sizeof(float), sizeof(float) * width));
    file.setFrameBuffer(fb);
    file.writePixels(height);

    spdlog::info("wrote HDR image to {}.exr", name);
  } catch (const std::exception &e) {
    spdlog::error("failed to write image to {}.exr: {}", name, e.what());
  }
}

// currently just clamps luminance (add tonemapping later)
auto write_sdr_image(std::vector<Vec3> const &img, uint32_t width,
                     uint32_t height, std::string const &name) -> void {
  std::vector<uint8_t> byte_data(width * height * 3);
  for (auto x = 0; x < width; x++) {
    for (auto y = 0; y < height; y++) {
      uint32_t px_idx = x + y * width;
      auto get_byte_val = [](float v) {
        return static_cast<uint8_t>(std::powf(std::min(v, 1.0f), 1.0f / 2.2f) *
                                    255.9f);
      };
      byte_data[3 * px_idx] = get_byte_val(img[px_idx].x);
      byte_data[3 * px_idx + 1] = get_byte_val(img[px_idx].y);
      byte_data[3 * px_idx + 2] = get_byte_val(img[px_idx].z);
    }
  }
  if (!stbi_write_png(fmt::format("{}.png", name).c_str(), width, height, 3,
                      reinterpret_cast<void *>(byte_data.data()), width * 3)) {
    spdlog::error("failed to write image to {}.png", name);
  } else {
    spdlog::info("wrote sdr image to {}.png", name);
  }
}
