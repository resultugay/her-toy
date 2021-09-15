#ifndef HER_SPAIR_H_
#define HER_SPAIR_H_
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "glog/logging.h"

namespace her {

template <typename VID_T>
struct VertexPair;

template <>
struct VertexPair<uint32_t> {
  using internal_t = uint64_t;
  VertexPair(uint32_t u, uint32_t v) : val(((internal_t) u << 32) | v) {}

  inline uint32_t u() const { return val >> 32; }

  inline uint32_t v() const { return static_cast<uint32_t>(val); }

  inline bool operator==(const VertexPair<uint32_t>& rhs) const {
    return val == rhs.val;
  }

  internal_t val;
};

template <>
struct VertexPair<uint64_t> {
  using internal_t = __int128;
  VertexPair(uint64_t u, uint64_t v) : val(((internal_t) u << 64) | v) {}

  inline uint64_t u() const { return val >> 64; }

  inline uint64_t v() const { return static_cast<uint64_t>(val); }

  inline bool operator==(const VertexPair<uint64_t>& rhs) const {
    return val == rhs.val;
  }

  internal_t val;
};

template <typename vertex_t>
class Cache {
 public:
  using key_t = VertexPair<vertex_t>;
  using candidates_t = std::unordered_set<key_t>;

  typedef struct val {
    bool match{};
    candidates_t candidate_matches;
  } val_t;

  void MarkUnmatchedAndClear(const key_t& k) {
    auto& val = cache_[k];

    val.match = false;
    val.candidate_matches.clear();
  }

  void MarkMatchedAndClear(const key_t& k) {
    auto& val = cache_[k];

    val.match = true;
    val.candidate_matches.clear();
  }

  bool SetMatchIfHit(const key_t& k, bool& match) {
    auto it = cache_.find(k);

    if (it != cache_.end()) {
      match = it->second.match;
      return true;
    }
    return false;
  }

  candidates_t& MarkMatchedAndReturn(const key_t& k) {
    auto& val = cache_[k];

    val.match = true;
    return val.candidate_matches;
  }

  void erase(const key_t& k) { cache_.erase(k); }

 private:
  std::unordered_map<key_t, val_t> cache_;
};

template <typename GRAPH, typename H_V, typename H_P, typename H_R>
class SPair {
  using vertex_t = typename GRAPH::vertex_t;
  using descendants_t = std::vector<std::pair<vertex_t, depth_t>>;
  using cache_t = Cache<vertex_t>;
  using key_t = typename cache_t::key_t;

 public:
  SPair(GRAPH& gd, GRAPH& g, H_V& h_v, H_P& h_p, H_R& h_r)
      : gd_(gd), g_(g), h_v_(h_v), h_p_(h_p), h_r_(h_r) {}

  ~SPair() {
    LOG(INFO) << "Max delta: " << seen_max_delta_
              << " Avg delta: " << total_delta_ / delta_count_;
  }

  void InitParams(double sigma, double delta, int k) {
    sigma_ = sigma;
    delta_ = delta;
    k_ = k;
  }

  bool Query(vertex_t u, vertex_t v) { return Query(u, v, 1); }

  bool Query(vertex_t u, vertex_t v, size_t curr_depth) {
    key_t key(u, v);
    bool match;

    if (cache_.SetMatchIfHit(key, match)) {
      return match;
    }

    if (curr_depth > 10) {
      return false;
    }

    auto sim = h_v_(gd_, u, g_, v);

    if (sim < sigma_) {
      cache_.MarkUnmatchedAndClear(key);
      return false;
    }

    // u is leaf node
    if (gd_.OutDegree(u) == 0) {
      cache_.MarkMatchedAndClear(key);
      return true;
    }

    // Generate Vuk
    if (ecache_gd_.find(u) == ecache_gd_.end()) {
      ecache_gd_[u] = h_r_(gd_, u, k_, false);
    }

    // Generate Vvk
    if (ecache_g_.find(v) == ecache_g_.end()) {
      ecache_g_[v] = h_r_(g_, v, k_, true);
    }

    auto& u_descendants = ecache_gd_.at(u);
    auto& v_descendants = ecache_g_.at(v);

    auto& W = cache_.MarkMatchedAndReturn(key);
    double sum = 0;

    W.clear();

    for (auto& u1_depth_pair : u_descendants) {
      vertex_t u1 = u1_depth_pair.first;
      depth_t u1_depth = u1_depth_pair.second;
      std::vector<std::pair<vertex_t, depth_t>> l;

      for (auto& v1_depth : v_descendants) {
        vertex_t v1 = v1_depth.first;

        if (h_v_(gd_, u1, g_, v1) >= sigma_) {
          l.push_back(v1_depth);
        }
      }

      // sort list by path score in descending order
      std::sort(l.begin(), l.end(),
                [this, u, u1, v](const std::pair<vertex_t, depth_t>& a,
                                 const std::pair<vertex_t, depth_t>& b) {
                  return h_p_(gd_, u, u1, g_, v, a.first) >
                         h_p_(gd_, u, u1, g_, v, b.first);
                });

      for (auto& v1_depth_pair : l) {
        vertex_t v1 = v1_depth_pair.first;
        depth_t v1_depth = v1_depth_pair.second;
        key_t key1(u1, v1);
        depth_t depth = std::max(u1_depth, v1_depth);

        if (!cache_.SetMatchIfHit(key1, match)) {
          match = Query(u1, v1, curr_depth + 1);
        }

        if (match) {
          sum += h_p_(gd_, u, u1, g_, v, v1) / depth;
          W.insert(key1);
          rev_cache_[key1].insert(key);

          seen_max_delta_ = std::max(seen_max_delta_, sum);
          total_delta_ += sum;
          delta_count_++;
          if (sum >= delta_) {
            return true;
          }
          break;
        }
      }
    }
    // Cleanup stage
    cache_.MarkUnmatchedAndClear(key);

    // need a copy to prevent modifications by recursive function call.
    auto rev_cache_copy = rev_cache_[key];
    // up, vp
    for (auto& key1 : rev_cache_copy) {
      auto up = key1.u();
      auto vp = key1.v();
      cache_.erase(key1);

      Query(up, vp, curr_depth + 1);
    }
    rev_cache_[key].clear();
    return false;
  }

  cache_t& cache() { return cache_; }

 private:
  GRAPH gd_;
  GRAPH g_;
  H_V h_v_;
  H_P h_p_;
  H_R h_r_;
  double sigma_{};
  double delta_{};
  int k_{};
  cache_t cache_;
  double seen_max_delta_{};
  double total_delta_{};
  uint64_t delta_count_{};
  std::unordered_map<key_t, std::unordered_set<key_t>> rev_cache_;
  std::unordered_map<vertex_t, descendants_t> ecache_gd_;
  std::unordered_map<vertex_t, descendants_t> ecache_g_;
};
}  // namespace her

template <typename VID_T>
struct std::hash<her::VertexPair<VID_T>> {
  inline std::size_t operator()(
      const her::VertexPair<VID_T>& pair) const noexcept {
    return pair.val;
  }
};

#endif  // HER_SPAIR_H_
