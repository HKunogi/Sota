#include "plain_mesh.h"

namespace sota {

void PlainMesh::calculate_final_heights(std::map<std::pair<int, int>, float>& distance_keeper, float diameter,
                                        int divisions) {
  shift_compress();
  _min_height += _y_shift;
  _min_height *= _y_compress;
  _min_height += _mesh->base().center().y;

  _max_height += _y_shift;
  _max_height *= _y_compress;
  _max_height += _mesh->base().center().y;
}

}  // namespace sota
