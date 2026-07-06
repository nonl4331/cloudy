#include "scene.hpp"
#include "coord.hpp"
#include <embree4/rtcore_buffer.h>
#include <embree4/rtcore_geometry.h>
#define TINYGLTF3_ENABLE_FS
#define TINYGLTF3_IMPLEMENTATION
#include "tiny_gltf_v3.h"
#include "vec.hpp"
#include <cstdint>
#include <embree4/rtcore_device.h>
#include <embree4/rtcore_scene.h>
#include <limits>
#include <spdlog/spdlog.h>
#include <vector>

auto Scene::lo(RTCRayHit ray, RTCIntersectArguments *args) -> Vec3 {
  auto out = vec3::ZERO;
  rtcIntersect1(scene, &ray, args);
  if (ray.hit.geomID == RTC_INVALID_GEOMETRY_ID) {
    out = sky.ray_value(ray);
  } else {
    Vec3(ray.hit.Ng_x, ray.hit.Ng_y, ray.hit.Ng_z).normalised();
    auto id = ray.hit.primID;
    auto u = ray.hit.u;
    auto v = ray.hit.v;
    auto uv = uv_buffer[index_buffer[id * 3]] * (1.0f - u - v) +
              uv_buffer[index_buffer[id * 3 + 1]] * u +
              uv_buffer[index_buffer[id * 3 + 2]] * v;
    out = Vec3(uv.x, uv.y, 0.0f).normalised();
  }
  return out;
}

Scene::Scene(Camera cam, Sky sky) : cam(cam), sky(sky) {
  RTCDevice device = rtcNewDevice(nullptr);
  RTCScene scene = rtcNewScene(device);
  rtcCommitScene(scene);
  this->device = device;
  this->scene = scene;
}

Scene::Scene(std::string const &file, uint32_t cam_idx, Camera cam, Sky sky)
    : cam(cam), sky(sky) {
  RTCDevice device = rtcNewDevice(nullptr);
  RTCScene scene = rtcNewScene(device);

  tg3_parse_options opts;
  tg3_error_stack errors;
  tg3_model model;

  tg3_parse_options_init(&opts);
  tg3_error_stack_init(&errors);

  tg3_error_code err =
      tg3_parse_file(&model, &errors, file.data(), file.size(), &opts);
  if (err != TG3_OK) {
    spdlog::info("{} parse did not return OK", file);
    for (int i = 0; i < errors.count; i++) {
      auto entry = errors.entries[i];
      auto msg = entry.message ? entry.message : "(null)";
      switch (entry.severity) {
      case tg3_severity::TG3_SEVERITY_INFO:
        spdlog::info("{} parse: {}", file, msg);
        break;
      case tg3_severity::TG3_SEVERITY_WARNING:
        spdlog::warn("{} parse: {}", file, msg);
        break;
      case tg3_severity::TG3_SEVERITY_ERROR:
        spdlog::error("{} parse: {}", file, msg);
        break;
      }
    }
  }

  float aspect = cam.get_aspect_ratio();
  if (model.cameras_count != 0) {
    if (cam_idx >= model.cameras_count) {
      spdlog::warn("camera idx {} does not exist (there are {} cameras) "
                   "failling back to camera 0",
                   cam_idx, model.cameras_count);
      cam_idx = 0;
    }
  }

  // shared index and vertex buffer between all geom
  std::vector<Vec3> vertex_buffer;
  std::vector<Vec3> normal_buffer;
  std::vector<Vec2> uv_buffer;
  std::vector<uint32_t> index_buffer;
  // index into material buffer
  std::vector<uint32_t> materials;

  auto default_scene = model.scenes[model.default_scene];

  // basically same design as
  // https://github.com/nonl4331/yapt/blob/main/yapt/src/loader.rs
  struct NodeCollection {
    std::vector<uint32_t> nodes;
    Vec3 translation;
    Quaternion rotation;
    Vec3 scale;
  };
  std::vector<uint32_t> nodes(default_scene.nodes,
                              default_scene.nodes + default_scene.nodes_count);
  std::vector<NodeCollection> node_queue = {
      NodeCollection{nodes, vec3::ZERO, quaternion::IDENTITY, vec3::ONE}};

  while (node_queue.size() != 0) {
    auto node_collection = node_queue.back();
    node_queue.pop_back();
    auto translation = node_collection.translation;
    auto rotation = node_collection.rotation;
    auto scale = node_collection.scale;
    while (node_collection.nodes.size() != 0) {
      auto node = model.nodes[node_collection.nodes.back()];
      node_collection.nodes.pop_back();

      auto local_translation =
          rotation
              .hamilton(
                  Quaternion(Vec3(node.translation[0], node.translation[1],
                                  node.translation[2])
                                 .hadamard(scale)))
              .hamilton(rotation.conj())
              .xyz() +
          translation;

      auto local_scale =
          scale.hadamard(Vec3(node.scale[0], node.scale[1], node.scale[2]));
      if (local_scale.x == 0.0f || local_scale.y == 0.0f ||
          local_scale.z == 0.0f) {
        spdlog::warn("local scale contains 0 value in glb parsing");
      }
      auto local_rotation =
          rotation.hamilton(Quaternion(node.rotation[3], node.rotation[0],
                                       node.rotation[1], node.rotation[2]));

      auto apply_transform = [&](Vec3 v) {
        v = v.hadamard(local_scale);

        return local_rotation.hamilton(Quaternion(v))
                   .hamilton(local_rotation.conj())
                   .xyz() +
               local_translation;
      };

      if (node.camera == cam_idx) {
        if (strcmp(model.cameras[cam_idx].type.data, "perspective") == 0) {
          spdlog::info("set camera");
          auto c = model.cameras[node.camera];
          cam = Camera(local_translation, local_rotation, c.perspective.yfov,
                       cam.get_width(), cam.get_height());
        } else {
          spdlog::warn("Orthographic cameras not supported yet. Tried to load "
                       "orthographic camera in {}. Using fallback camera.",
                       file);
        }
      }

      if (node.mesh != -1) {
        auto mesh = model.meshes[node.mesh];
        std::vector<tg3_primitive> primitives(
            mesh.primitives, mesh.primitives + mesh.primitives_count);
        for (auto primitive : primitives) {
          if (primitive.mode != TG3_MODE_TRIANGLES && primitive.mode != -1) {
            continue;
          }

          uint32_t vert_offset = vertex_buffer.size();
          uint32_t norm_offset = normal_buffer.size();

          if (primitive.indices == -1) {
            spdlog::critical("Missing expected indicies in glb loading");
          }

          uint32_t pos_idx = std::numeric_limits<uint32_t>().max();
          uint32_t nor_idx = std::numeric_limits<uint32_t>().max();
          uint32_t uv_idx = std::numeric_limits<uint32_t>().max();
          for (uint32_t i = 0; i < primitive.attributes_count; i++) {
            auto key = primitive.attributes[i].key.data;
            if (strcmp(key, "POSITION") == 0) {
              pos_idx = primitive.attributes[i].value;
            } else if (strcmp(key, "NORMAL") == 0) {
              nor_idx = primitive.attributes[i].value;
            } else if (strcmp(key, "TEXCOORD_0") == 0) {
              uv_idx = primitive.attributes[i].value;
            }
          }

          // positions
          if (pos_idx == std::numeric_limits<uint32_t>().max()) {
            spdlog::critical("Missing position attribute in glb loading");
          }
          auto pos_acc = model.accessors[pos_idx];
          auto pos_view = model.buffer_views[pos_acc.buffer_view];
          auto pos_buf = model.buffers[pos_view.buffer];
          auto pos_stride = pos_view.byte_stride > 0 ? pos_view.byte_stride
                                                     : 3 * sizeof(float);
          for (uint32_t v = 0; v < pos_acc.count; v++) {
            auto vec = reinterpret_cast<const float *>(
                pos_buf.data.data + pos_acc.byte_offset + pos_view.byte_offset +
                v * pos_stride);
            auto vert = apply_transform(Vec3(vec[0], vec[1], vec[2]));
            vertex_buffer.push_back(vert);
          }

          // normals
          if (nor_idx == std::numeric_limits<uint32_t>().max()) {
            spdlog::critical("Missing normal attribute in glb loading");
          }
          auto nor_acc = model.accessors[nor_idx];
          auto nor_view = model.buffer_views[nor_acc.buffer_view];
          auto nor_buf = model.buffers[nor_view.buffer];
          auto nor_stride = nor_view.byte_stride > 0 ? nor_view.byte_stride
                                                     : 3 * sizeof(float);
          for (uint32_t v = 0; v < nor_acc.count; v++) {
            auto vec = reinterpret_cast<const float *>(
                nor_buf.data.data + nor_acc.byte_offset + nor_view.byte_offset +
                v * nor_stride);
            normal_buffer.push_back(
                local_rotation
                    .hamilton(Quaternion(Vec3(vec[0], vec[1], vec[2])))
                    .hamilton(local_rotation.conj())
                    .xyz()
                    .normalised());
          }

          // uvs
          if (uv_idx == std::numeric_limits<uint32_t>().max()) {
            spdlog::critical("Missing uv attribute in glb loading");
          }
          auto uv_acc = model.accessors[uv_idx];
          auto uv_view = model.buffer_views[uv_acc.buffer_view];
          auto uv_buf = model.buffers[uv_view.buffer];
          auto uv_stride =
              uv_view.byte_stride > 0 ? uv_view.byte_stride : 2 * sizeof(float);
          for (uint32_t v = 0; v < uv_acc.count; v++) {
            auto vec = reinterpret_cast<const float *>(
                uv_buf.data.data + uv_acc.byte_offset + uv_view.byte_offset +
                v * uv_stride);
            uv_buffer.push_back(Vec2(vec[0], vec[1]));
          }

          // indicies
          auto ind_acc = model.accessors[primitive.indices];
          auto ind_view = model.buffer_views[ind_acc.buffer_view];
          auto ind_buf = model.buffers[ind_view.buffer];
          auto ind_stride = ind_view.byte_stride;
          for (uint32_t v = 0; v < ind_acc.count; v++) {
            uint32_t index = 0;
            switch (ind_acc.component_type) {
            case TG3_COMPONENT_TYPE_UNSIGNED_INT:
              ind_stride = ind_stride > 0 ? ind_stride : sizeof(uint32_t);
              index = reinterpret_cast<const uint32_t *>(
                  ind_buf.data.data + ind_acc.byte_offset +
                  ind_view.byte_offset + v * ind_stride)[0];
              break;
            case TG3_COMPONENT_TYPE_UNSIGNED_SHORT:
              ind_stride = ind_stride > 0 ? ind_stride : sizeof(uint16_t);
              index = reinterpret_cast<const uint16_t *>(
                  ind_buf.data.data + ind_acc.byte_offset +
                  ind_view.byte_offset + v * ind_stride)[0];
              break;
            case TG3_COMPONENT_TYPE_UNSIGNED_BYTE:
              ind_stride = ind_stride > 0 ? ind_stride : sizeof(uint8_t);
              index = reinterpret_cast<const uint8_t *>(
                  ind_buf.data.data + ind_acc.byte_offset +
                  ind_view.byte_offset + v * ind_stride)[0];
              break;
            default:
              spdlog::critical("Unexpected index type in glb loading");
              break;
            };
            index_buffer.push_back(index + vert_offset);
          }
        }
      }

      if (node.children_count != 0) {
        auto child_nodes = node.children;
        std::vector<uint32_t> new_nodes(node.children,
                                        node.children + node.children_count);
        node_queue.push_back(NodeCollection{new_nodes, local_translation,
                                            local_rotation, local_scale});
      }
    }
  }

  // TODO: load materials
  this->cam = cam;
  this->vertex_buffer = std::move(vertex_buffer);
  this->normal_buffer = std::move(normal_buffer);
  this->uv_buffer = std::move(uv_buffer);
  this->index_buffer = std::move(index_buffer);
  this->materials = std::move(materials);

  spdlog::info("Loaded: {} triangles", this->index_buffer.size() / 3);
  spdlog::info("Loaded: {} verts", this->vertex_buffer.size());
  spdlog::info("Loaded: {} normals", this->normal_buffer.size());

  RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

  RTCBuffer vert_buf =
      rtcNewSharedBufferHostDevice(device, this->vertex_buffer.data(),
                                   sizeof(Vec3) * this->vertex_buffer.size());
  rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3,
                       vert_buf, 0, sizeof(Vec3), this->vertex_buffer.size());

  RTCBuffer norm_buf =
      rtcNewSharedBufferHostDevice(device, this->normal_buffer.data(),
                                   sizeof(Vec3) * this->normal_buffer.size());
  rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_NORMAL, 0, RTC_FORMAT_FLOAT3,
                       norm_buf, 0, sizeof(Vec3), this->normal_buffer.size());

  RTCBuffer ind_buf = rtcNewSharedBufferHostDevice(
      device, this->index_buffer.data(),
      sizeof(uint32_t) * this->index_buffer.size());
  rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3,
                       ind_buf, 0, 3 * sizeof(uint32_t),
                       this->index_buffer.size() / 3);

  rtcCommitGeometry(geom);
  rtcAttachGeometry(scene, geom);
  rtcReleaseGeometry(geom);
  rtcCommitScene(scene);

  tg3_model_free(&model);
  tg3_error_stack_free(&errors);

  this->device = device;
  this->scene = scene;
}
