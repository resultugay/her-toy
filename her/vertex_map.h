#ifndef HER_VERTEX_MAP_H_
#define HER_VERTEX_MAP_H_
#include <unordered_map>

namespace her {
template <typename OID_T, typename VID_T>
class VertexMap {
 public:
  bool AddVertex(const OID_T& oid, VID_T& lid) {
    auto it = o2l_.find(oid);

    if (it == o2l_.end()) {
      lid = l2o_.size();

      o2l_[oid] = lid;
      l2o_.push_back(oid);
      return true;
    } else {
      lid = it->second;
      return false;
    }
  }

  bool GetLid(const OID_T& oid, VID_T& lid) const {
    auto it = o2l_.find(oid);

    if (it == o2l_.end()) {
      return false;
    }

    lid = it->second;
    return true;
  }

  bool GetOid(const VID_T& lid, OID_T& oid) const {
    if (lid < l2o_.size()) {
      oid = l2o_[lid];
      return true;
    }
    return false;
  }

  VID_T TotalVertexNum() const { return o2l_.size(); }

  bool HasOid(const OID_T& oid) const { return o2l_.find(oid) != o2l_.end(); }

 private:
  std::unordered_map<OID_T, VID_T> o2l_;
  std::vector<OID_T> l2o_;
};

}  // namespace her
#endif  // HER_VERTEX_MAP_H_
