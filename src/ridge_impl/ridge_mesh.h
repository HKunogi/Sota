#pragma once

#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <vector>

#include "core/hex_mesh.h"
#include "general_utility.h"
#include "mesh.h"
#include "misc/types.h"
#include "pent_mesh.h"
#include "primitives/pentagon.h"
#include "ridge_impl/ridge.h"
#include "tal/arrays.h"
#include "tal/noise.h"
#include "tal/reference.h"
#include "tal/vector3.h"
#include "utils.h"

namespace sota {

struct RidgeHexMeshParams {
  HexMeshParams hex_mesh_params;
  Ref<FastNoiseLite> plain_noise{nullptr};
  Ref<FastNoiseLite> ridge_noise{nullptr};
};

struct RidgePentagonMeshParams {
  PentagonMeshParams pentagon_mesh_params;
  Ref<FastNoiseLite> plain_noise{nullptr};
  Ref<FastNoiseLite> ridge_noise{nullptr};
};

class RidgeMesh : public TileMesh {
  GDCLASS(RidgeMesh, TileMesh)
 public:
  RidgeMesh() = default;  // existence is 'must' for Godot
  RidgeMesh(const RidgeMesh& other) = delete;
  RidgeMesh(RidgeMesh&& other) = delete;
  // copying operator= defined inside GDCLASS
  RidgeMesh& operator=(RidgeMesh&& rhs) = delete;

  // getters
  std::vector<TileMesh*> get_neighbours() const;
  GroupedMeshVertices get_grouped_vertices();
  std::pair<float, float> get_min_max_height() const { return {_min_height, _max_height}; }

  // setters
  void set_plain_noise(Ref<FastNoiseLite> plain_noise);
  void set_ridge_noise(Ref<FastNoiseLite> ridge_noise);
  void set_neighbours(Neighbours p_neighbours) { _neighbours = p_neighbours; }
  void set_ridges(std::vector<Ridge*> r) { _ridges = r; }
  void set_shift_compress(float y_shift, float y_compress);

  // calculation
  void calculate_corner_points_distances_to_border(std::map<std::pair<int, int>, float>& distance_keeper,
                                                   int divisions);
  void calculate_initial_heights();
  virtual void calculate_final_heights(std::map<std::pair<int, int>, float>& distance_keeper, float diameter,
                                       int divisions) = 0;

  void calculate_normals() { _mesh->calculate_normals(); }
  void update() { _mesh->update(); }
  void recalculate_all_except_vertices() { _mesh->recalculate_all_except_vertices(); }
  void init() { _mesh->init(); }
  Vector3 get_center() { return _mesh->get_center(); }
  SotaMesh* inner_mesh() override { return _mesh.ptr(); }

  int get_id() override { return _mesh->get_id(); }

 protected:
  RidgeMesh(Hexagon hex, RidgeHexMeshParams params)
      : _mesh(Ref<SotaMesh>(memnew(HexMesh(hex, params.hex_mesh_params)))) {
    _plain_noise = params.plain_noise;
    _ridge_noise = params.ridge_noise;
  }

  RidgeMesh(Pentagon pentagon, RidgePentagonMeshParams params)
      : _mesh(Ref<SotaMesh>(memnew(PentMesh(pentagon, params.pentagon_mesh_params)))) {
    _plain_noise = params.plain_noise;
    _ridge_noise = params.ridge_noise;
  }

  static void _bind_methods();

  Vector3Array _initial_vertices;

  void shift_compress();
  void calculate_ridge_based_heights(std::function<double(double, double, double)> interpolation_func,
                                     float ridge_offset, std::map<std::pair<int, int>, float>& distance_keeper,
                                     int divisions);

  Ref<FastNoiseLite> _plain_noise;
  Ref<FastNoiseLite> _ridge_noise;
  std::vector<Ridge*> _ridges;
  Neighbours _neighbours = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

  float _min_height = std::numeric_limits<float>::max();
  float _max_height = std::numeric_limits<float>::min();
  float _y_shift = 0.0f;     // no shift
  float _y_compress = 1.0f;  // no compress

  Ref<SotaMesh> _mesh;
  std::unique_ptr<MeshProcessor> _processor;

 private:
  template <typename T>
  friend Ref<RidgeMesh> make_ridge_pentagon_mesh(Pentagon pentagon, RidgePentagonMeshParams params);

  template <typename T>
  friend Ref<RidgeMesh> make_ridge_hex_mesh(Hexagon hex, RidgeHexMeshParams params);

  // TODO make const. inner_mesh have to be const?
  std::set<int> get_exclude_list();
};

template <typename T>
Ref<RidgeMesh> make_ridge_hex_mesh(Hexagon hex, RidgeHexMeshParams params) {
  auto res = Ref<T>(memnew(T(hex, params)));
  res->init();
  if (params.hex_mesh_params.orientation == Orientation::Plane) {
    res->_processor = std::make_unique<FlatMeshProcessor>(FlatMeshProcessor());
  } else {
    res->_processor = std::make_unique<VolumeMeshProcessor>(VolumeMeshProcessor(res->inner_mesh()->get_vertices()));
  }
  return res;
}

template <typename T>
Ref<RidgeMesh> make_ridge_pentagon_mesh(Pentagon pentagon, RidgePentagonMeshParams params) {
  auto res = Ref<T>(memnew(T(pentagon, params)));
  res->init();
  if (params.pentagon_mesh_params.orientation == Orientation::Plane) {
    res->_processor = std::make_unique<FlatMeshProcessor>(FlatMeshProcessor());
  } else {
    res->_processor = std::make_unique<VolumeMeshProcessor>(VolumeMeshProcessor(res->inner_mesh()->get_vertices()));
  }
  return res;
}

}  // namespace sota
