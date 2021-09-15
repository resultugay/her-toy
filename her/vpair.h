#ifndef PARAMATRICSIMULATION_HER_VPAIR_H_
#define PARAMATRICSIMULATION_HER_VPAIR_H_

#include "her/spair.h"
namespace her {
template <typename GRAPH, typename H_V, typename H_P, typename H_R>
class VPair {
  using vertex_t = typename GRAPH::vertex_t;

 public:
  VPair(GRAPH& gd, GRAPH& g, H_V& h_v, H_P& h_p, H_R& h_r)
      : gd_(gd), g_(g), h_v_(h_v), s_pair_(gd, g, h_v, h_p, h_r) {}

  void InitParams(double sigma, double delta, int k) {
    s_pair_.InitParams(sigma, delta, k);
    sigma_ = sigma;
  }

  std::vector<vertex_t> Query(vertex_t u) {
    std::vector<vertex_t> result;
    auto vertices = g_.Vertices();
    std::vector<vertex_t> C;

    auto begin = GetCurrentTime();
    for (vertex_t v : vertices) {
      if (h_v_(gd_, u, g_, v) >= sigma_) {
        C.push_back(v);
      }
    }
    LOG(INFO) << "Calculate candidate: " << GetCurrentTime() - begin << " s"
              << " C size: " << C.size();

    // sort vertices of g in increasing order of degree
    std::sort(C.begin(), C.end(), [this](const vertex_t& a, const vertex_t& b) {
      return g_.OutDegree(a) < g_.OutDegree(b);
    });

    auto& cache = s_pair_.cache();

    begin = GetCurrentTime();

    for (vertex_t v : C) {
      bool match;

      // issue a query if not hit cache
      if (!cache.SetMatchIfHit(VertexPair<vertex_t>(u, v), match)) {
        match = s_pair_.Query(u, v);
      }

      if (match) {
        result.push_back(v);
      }
    }
    LOG(INFO) << "SPair query: " << GetCurrentTime() - begin << " s";
    return result;
  }

 private:
  GRAPH gd_;
  GRAPH g_;
  H_V& h_v_;
  double sigma_{};
  SPair<GRAPH, H_V, H_P, H_R> s_pair_;
};
}  // namespace her
#endif  // PARAMATRICSIMULATION_HER_VPAIR_H_
