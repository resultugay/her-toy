#ifndef PARAMATRICSIMULATION_HER_PROCESSING_UTILS_H_
#define PARAMATRICSIMULATION_HER_PROCESSING_UTILS_H_
#include <immintrin.h>

#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "her/config.h"

namespace her {
template <typename T>
inline T CosineSimilarity(const dense_vector_t<T>& A,
                          const dense_vector_t<T>& B) {
  if (A.size() == 0 || B.size() == 0)
    return 0;
  return A.dot(B) / (std::sqrt(A.dot(A)) * std::sqrt(B.dot(B)));
}

template <typename T>
inline dense_vector_t<T> TextToVector(
    const std::unordered_map<std::string, dense_vector_t<T>>& word_embeddings,
    const std::string& text) {
  //  auto unknown_it = word_embeddings.find("unk");
  size_t word_count = 0, matched_count = 0;
  dense_vector_t<T> vector;
  std::vector<std::string> str_vec;
  boost::algorithm::split(str_vec, text, boost::is_any_of("\t ,;|"),
                          boost::token_compress_on);

  // word-wise adding
  for (auto& token : str_vec) {
    if (!token.empty()) {
      auto it = word_embeddings.find(token);

      // unknown word
      if (it != word_embeddings.end()) {
        matched_count++;
        auto& vec_of_word = it->second;

        if (vector.size() == 0) {
          vector.resize(vec_of_word.size());
          vector.fill(0);
        }
        vector += vec_of_word;
      }
      word_count++;
    }
  }

  if (vector.size() == 0) {
    return vector;
  }

  //  if (unknown_it != word_embeddings.end() && matched_count != word_count) {
  //    vector += unknown_it->second;
  //  }

  // Average
  vector /= word_count;

  return vector;
}

template <typename VECTOR_T, typename GRAPH_T>
void FillWordVector(
    const GRAPH_T& g,
    const std::unordered_map<std::string, VECTOR_T>& word_embedding,
    VertexArray<VECTOR_T, GRAPH_T>& word_vector, int parallelism) {
  auto vertices = g.Vertices();

  word_vector.Init(vertices);

  std::vector<std::thread> threads;
  auto ranges = vertices.ToChunks(parallelism);

  for (auto sub_range : ranges) {
    threads.push_back(std::thread(
        [&g, &word_embedding,
         &word_vector](typename GRAPH_T::vertex_range_t range) {
          for (auto v : range) {
            auto& label = g[v];

            word_vector[v] = TextToVector(word_embedding, label);
          }
        },
        sub_range));
  }

  for (auto& th : threads) {
    th.join();
  }
}

template <typename COORD_T, typename GRAPH_T>
std::vector<dense_vector_t<COORD_T>> ExtractPoints(
    const std::unordered_map<std::string, dense_vector_t<COORD_T>>&
        word_embeddings,
    const GRAPH_T& g, const typename GRAPH_T::vertex_range_t& vertices) {
  std::vector<dense_vector_t<COORD_T>> points;

  for (auto v : vertices) {
    auto& label = g[v];
    auto point = TextToVector(word_embeddings, label);

    if (point.size() > 0) {
      points.push_back(point);
    }
  }

  return points;
}

template <typename GRAPH_T>
inline std::vector<std::pair<typename GRAPH_T::vertex_t, depth_t>> BFS(
    const GRAPH_T& g, typename GRAPH_T::vertex_t src, depth_t depth_limit,
    size_t k = std::numeric_limits<size_t>::max()) {
  depth_t curr_depth = 0;
  std::queue<typename GRAPH_T::vertex_t> queue;
  std::unordered_set<typename GRAPH_T::vertex_t> visited;
  std::vector<std::pair<typename GRAPH_T::vertex_t, depth_t>> descendants;

  queue.push(src);

  while (curr_depth++ < depth_limit) {
    auto size = queue.size();

    while (size-- > 0) {
      auto u = queue.front();
      queue.pop();

      auto oes = g.GetOutgoingAdjList(u);
      for (auto& e : oes) {
        auto v = g.target(e);

        if (visited.find(v) == visited.end()) {
          queue.push(v);
          descendants.template emplace_back(v, curr_depth);
          visited.insert(v);

          if (descendants.size() >= k) {
            return descendants;
          }
        }
      }
    }
    curr_depth++;
  }

  return descendants;
}

template <typename T>
inline std::vector<std::pair<typename std::vector<T>::const_iterator,
                             typename std::vector<T>::const_iterator>>
ToChunks(const std::vector<T>& vec, size_t n_chunks) {
  std::vector<std::pair<typename std::vector<T>::const_iterator,
                        typename std::vector<T>::const_iterator>>
      chunks;
  size_t size = vec.size();
  size_t chunk_size = size / n_chunks;

  CHECK_GT(chunk_size, 0) << "Too many chunks";

  for (size_t i = 0; i < n_chunks; i++) {
    auto begin = vec.begin() + i * chunk_size;
    auto end = vec.begin() + (i + 1) * chunk_size;

    if (i == n_chunks - 1) {
      end = vec.end();
    }

    chunks.template emplace_back(begin, end);
  }

  return chunks;
}

template <typename GRAPH_T>
inline std::string ConcatEdgeLabel(const GRAPH_T& g,
                                   typename GRAPH_T::vertex_t src,
                                   typename GRAPH_T::vertex_t dst,
                                   const std::string& delimiter) {
  using vertex_t = typename GRAPH_T::vertex_t;
  using edge_t = typename GRAPH_T::edge_t;
  std::queue<std::pair<vertex_t, std::vector<edge_t>>> queue;
  std::unordered_set<vertex_t> visited;

  queue.push(std::make_pair(src, std::vector<edge_t>()));

  while (!queue.empty()) {
    auto pair = queue.front();
    auto u = pair.first;
    auto& src_path = pair.second;
    auto oes = g.GetOutgoingAdjList(u);

    for (auto& oe : oes) {
      auto v = g.target(oe);

      if (visited.find(v) == visited.end()) {
        visited.insert(v);
        queue.template emplace(v, src_path);

        auto& dst_path = queue.back().second;

        dst_path.push_back(oe);

        if (v == dst) {
          std::vector<std::string> tokens(dst_path.size());

          for (size_t i = 0; i < dst_path.size(); i++) {
            auto& e = dst_path[i];

            tokens[i] = g[e];
          }

          return boost::algorithm::join(tokens, delimiter);
        }
      }
    }

    queue.pop();
  }

  return {};
}
}  // namespace her

#endif  // PARAMATRICSIMULATION_HER_PROCESSING_UTILS_H_
