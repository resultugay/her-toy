#ifndef PARAMATRICSIMULATION_HER_APAIR_PARALLEL_H_
#define PARAMATRICSIMULATION_HER_APAIR_PARALLEL_H_
#include <mutex>
#include <random>

#include "boost/serialization/vector.hpp"
#include "her/config.h"
#include "her/graph.h"
#include "her/inverted_index.h"
#include "her/processing_utils.h"
#include "her/spair.h"
#include "her/util.h"

namespace her {

template <typename GRAPH, typename COORD_T, typename H_V, typename H_P,
          typename H_R>
class APairParallel {
  using vertex_t = typename GRAPH::vertex_t;

 public:
  APairParallel(GRAPH& gd, GRAPH& g, H_V& h_v, H_P& h_p, H_R& h_r,
                const std::unordered_set<std::string>& gd_source_labels,
                const std::unordered_set<std::string>& g_source_labels,
                const InvertedIndex<GRAPH>& inverted_index)
      : gd_(gd),
        g_(g),
        h_v_(h_v),
        s_pair_(gd, g, h_v, h_p, h_r),
        gd_source_labels_(gd_source_labels),
        g_source_labels_(g_source_labels),
        inverted_index_(inverted_index) {}

  void InitParams(double sigma, double delta, int k, int parallelism) {
    s_pair_.InitParams(sigma, delta, k);
    sigma_ = sigma;
    parallelism_ = parallelism;
  }

  std::vector<VertexPair<vertex_t>> Query() {
    std::vector<std::pair<vertex_t, std::vector<vertex_t>>> C;
    boost::mpi::communicator world;
    int rank = world.rank();

    {
      auto begin = GetCurrentTime();

      std::vector<vertex_t> vertices;

      if (world.rank() == 0) {
        for (auto v : gd_.Vertices()) {
          vertices.push_back(v);
        }

        auto rng = std::default_random_engine{};
        rng.seed(0);
        std::shuffle(std::begin(vertices), std::end(vertices), rng);
      }

      boost::mpi::broadcast(world, vertices, 0);

      auto n_proc = world.size();
      auto local_vertices_bound = ToChunks(vertices, n_proc)[world.rank()];
      std::vector<vertex_t> local_vertices(local_vertices_bound.first,
                                           local_vertices_bound.second);
      std::vector<std::thread> threads;
      std::mutex mutex;

      for (auto thread_work_bound : ToChunks(local_vertices, parallelism_)) {
        threads.push_back(std::thread(
            [this, &mutex, &C, rank](
                const std::pair<typename std::vector<vertex_t>::const_iterator,
                                typename std::vector<vertex_t>::const_iterator>&
                    thread_work_bound) {
              std::vector<vertex_t> vertices_of_gd(thread_work_bound.first,
                                                   thread_work_bound.second);
              std::vector<std::pair<vertex_t, std::vector<vertex_t>>> local_C;
              auto query_begin = GetCurrentTime();
              size_t seen_n_points = 0;

              for (auto u : vertices_of_gd) {
                if (gd_.OutDegree(u) == 0) {
                  continue;
                }
                auto& u_label = gd_[u];

                if (gd_source_labels_.find(u_label) !=
                    gd_source_labels_.end()) {
                  std::vector<vertex_t> vertices;  // vertices of g

                  if (seen_n_points++ % 500 == 0) {
                    VLOG(2) << "Rank: " << rank << " "
                            << (GetCurrentTime() - query_begin) / seen_n_points
                            << " seconds/point";
                  }
                  auto v_list = inverted_index_.Query(u_label);

                  for (auto& v : v_list) {
                    if (g_.OutDegree(v) == 0) {
                      continue;
                    }
                    auto& v_label = g_[v];

                    if (g_source_labels_.find(v_label) !=
                        g_source_labels_.end()) {
                      // We filter vertex pair before query
                      if (h_v_(gd_, u, g_, v) >= sigma_) {
                        vertices.push_back(v);
                      }
                    }
                  }

                  if (!vertices.empty()) {
                    local_C.template emplace_back(u, vertices);
                  }
                }
              }

              {
                // Sort C by outdegree
                for (auto& pair : local_C) {
                  auto& vertices = pair.second;

                  std::sort(vertices.begin(), vertices.end(),
                            [this](const vertex_t& a, const vertex_t& b) {
                              return g_.OutDegree(a) < g_.OutDegree(b);
                            });
                }

                std::lock_guard<std::mutex> lock(mutex);

                C.insert(std::end(C), std::begin(local_C), std::end(local_C));
              }
            },
            thread_work_bound));
      }

      for (auto& th : threads) {
        th.join();
      }

      LOG(INFO) << "Rank: " << rank
                << " Generate candidates: " << GetCurrentTime() - begin
                << " s, local candidates: " << C.size();
    }

    // This is not necessary, but we add this to make timing more accurate
    world.barrier();

    auto& cache = s_pair_.cache();
    std::vector<VertexPair<vertex_t>> result;

    size_t seen_n_points = 0;
    {
      auto begin = GetCurrentTime();
      size_t n_pairs = 0;

      for (auto& pair : C) {
        auto u = pair.first;
        auto& vertices = pair.second;

        n_pairs += vertices.size();
        for (auto v : vertices) {
          bool match;

          // issue a query if not hit cache
          if (!cache.SetMatchIfHit(VertexPair<vertex_t>(u, v), match)) {
            match = s_pair_.Query(u, v);
          }

          if (match) {
            result.template emplace_back(u, v);
          }
          if (seen_n_points++ % 1000 == 0) {
            VLOG(10) << "Finished query " << gd_.GetId(u) << " " << g_.GetId(v)
                     << " finished, ans: " << match;
          }
        }
      }
      LOG(INFO) << "Rank: " << world.rank() << " |pairs|: " << n_pairs
                << " SPair query: " << GetCurrentTime() - begin << " s";
    }
    return result;
  }

 private:
  GRAPH gd_;
  GRAPH g_;
  H_V& h_v_;
  double sigma_{};
  int parallelism_{};
  SPair<GRAPH, H_V, H_P, H_R> s_pair_;
  const std::unordered_set<std::string>& gd_source_labels_;
  const std::unordered_set<std::string>& g_source_labels_;
  const InvertedIndex<GRAPH>& inverted_index_;
};
}  // namespace her

#endif  // PARAMATRICSIMULATION_HER_APAIR_PARALLEL_H_
