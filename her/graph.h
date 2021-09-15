#ifndef HER_GRAPH_H_
#define HER_GRAPH_H_
#include <boost/graph/compressed_sparse_row_graph.hpp>

#include "glog/logging.h"
#include "her/vertex_map.h"

namespace her {
enum class LoadStrategy { kOnlyIn, kOnlyOut, kBothOutIn };

template <typename VID_T, typename VDATA_T, typename EDATA_T,
          LoadStrategy load_strategy>
struct boost_graph;

template <typename VID_T, typename VDATA_T, typename EDATA_T>
struct boost_graph<VID_T, VDATA_T, EDATA_T, LoadStrategy::kOnlyOut> {
  using type = boost::compressed_sparse_row_graph<boost::directedS, VDATA_T,
                                                  EDATA_T, boost::no_property,
                                                  VID_T, std::size_t>;
};

template <typename VID_T, typename VDATA_T, typename EDATA_T>
struct boost_graph<VID_T, VDATA_T, EDATA_T, LoadStrategy::kBothOutIn> {
  using type =
      boost::compressed_sparse_row_graph<boost::bidirectionalS, VDATA_T,
                                         EDATA_T, boost::no_property, VID_T,
                                         std::size_t>;
};

template <typename BOOST_GRAPH>
class VertexRange {
  using graph_t = BOOST_GRAPH;

 public:
  VertexRange() = default;

  explicit VertexRange(
      const std::pair<typename boost::graph_traits<graph_t>::vertex_iterator,
                      typename boost::graph_traits<graph_t>::vertex_iterator>&
          vertices)
      : begin_(vertices.first), end_(vertices.second) {}

  explicit VertexRange(
      const typename boost::graph_traits<graph_t>::vertex_iterator& begin,
      const typename boost::graph_traits<graph_t>::vertex_iterator& end)
      : begin_(begin), end_(end) {}

  typename boost::graph_traits<graph_t>::vertex_iterator begin() {
    return begin_;
  }

  typename boost::graph_traits<graph_t>::vertex_iterator end() { return end_; }

  typename boost::graph_traits<graph_t>::vertex_iterator begin() const {
    return begin_;
  }

  typename boost::graph_traits<graph_t>::vertex_iterator end() const {
    return end_;
  }

  size_t size() const { return end_ - begin_; }

  std::vector<VertexRange<BOOST_GRAPH>> ToChunks(size_t n_chunks) {
    std::vector<VertexRange<BOOST_GRAPH>> chunks;
    size_t size = end_ - begin_;
    size_t chunk_size = size / n_chunks;

    CHECK_GT(chunk_size, 0) << "Too many chunks";

    for (size_t i = 0; i < n_chunks; i++) {
      auto begin = begin_ + i * chunk_size;
      auto end = begin_ + (i + 1) * chunk_size;

      if (i == n_chunks - 1) {
        end = end_;
      }

      chunks.template emplace_back(begin, end);
    }

    return chunks;
  }

 private:
  typename boost::graph_traits<graph_t>::vertex_iterator begin_;
  typename boost::graph_traits<graph_t>::vertex_iterator end_;
};

template <typename T, typename GRAPH_T>
class VertexArray {
  using boost_graph_t = typename GRAPH_T::boost_graph_t;
  using vertex_t =
      typename boost::graph_traits<boost_graph_t>::vertex_descriptor;

 public:
  VertexArray() = default;

  explicit VertexArray(const VertexRange<boost_graph_t>& range)
      : data_(range.size()), range_(range) {
    fake_start_ = data_.data() - range_.begin().base();
  }

  VertexArray(const VertexRange<boost_graph_t>& range, const T& value)
      : data_(range.size(), value), range_(range) {
    fake_start_ = data_.data() - range_.begin().base();
  }

  void Init(const VertexRange<boost_graph_t>& range) {
    data_.clear();
    data_.resize(range.size());
    range_ = range;
    fake_start_ = data_.data() - range_.begin().base();
  }

  void Init(const VertexRange<boost_graph_t>& range, const T& value) {
    data_.clear();
    data_.resize(range.size(), value);
    range_ = range;
    fake_start_ = data_.data() - range_.begin().base();
  }

  void SetValue(const VertexRange<boost_graph_t>& range, const T& value) {
    std::fill_n(&data_[range.begin().base() - range_.begin().base()],
                range.size(), value);
  }

  void SetValue(const T& value) { std::fill_n(data_, data_.size(), value); }

  inline T& operator[](const vertex_t& loc) { return fake_start_[loc]; }

  inline const T& operator[](const vertex_t& loc) const {
    return fake_start_[loc];
  }

  const VertexRange<boost_graph_t>& GetVertexRange() const { return range_; }

 private:
  VertexRange<boost_graph_t> range_;
  std::vector<T> data_;
  T* fake_start_;
};

template <typename EDGE_ITERATOR>
class AdjList {
 public:
  explicit AdjList(const std::pair<EDGE_ITERATOR, EDGE_ITERATOR>& edges)
      : begin_(edges.first), end_(edges.second) {}

  EDGE_ITERATOR begin() { return begin_; }

  EDGE_ITERATOR end() { return end_; }

 private:
  EDGE_ITERATOR begin_;
  EDGE_ITERATOR end_;
};

template <typename OID_T, typename VID_T, typename VDATA_T, typename EDATA_T,
          LoadStrategy _load_strategy = LoadStrategy::kOnlyOut>
class Graph {
 public:
  using oid_t = OID_T;
  using vid_t = VID_T;
  using vdata_t = VDATA_T;
  using edata_t = EDATA_T;
  using boost_graph_t =
      typename boost_graph<vid_t, vdata_t, edata_t, _load_strategy>::type;
  using vertex_t =
      typename boost::graph_traits<boost_graph_t>::vertex_descriptor;
  using vertex_range_t = VertexRange<boost_graph_t>;
  using edge_t = typename boost::graph_traits<boost_graph_t>::edge_descriptor;
  using vertex_map_t = VertexMap<oid_t, vid_t>;

  static constexpr LoadStrategy load_strategy = _load_strategy;
  static_assert(std::is_same<vertex_t, vid_t>::value,
                "Unexpected type assertion");

  Graph() = default;

  Graph(std::shared_ptr<vertex_map_t> vm_ptr,
        std::shared_ptr<boost_graph_t> graph)
      : vertex_map_(vm_ptr), graph_(graph) {}

  vertex_range_t Vertices() const {
    return vertex_range_t(boost::vertices(*graph_));
  }

  bool GetId(const vertex_t& v, oid_t& oid) const {
    return vertex_map_->GetOid(v, oid);
  }

  oid_t GetId(const vertex_t& v) const {
    oid_t oid;
    CHECK(vertex_map_->GetOid(v, oid));
    return oid;
  }

  bool GetVertex(const oid_t& oid, vertex_t& vertex) const {
    return vertex_map_->GetLid(oid, vertex);
  }

  const vdata_t& operator[](const vertex_t& v) const {
    return graph_->operator[](v);
  }

  vdata_t& operator[](const vertex_t& v) { return graph_->operator[](v); }

  AdjList<typename boost_graph_t::out_edge_iterator> GetOutgoingAdjList(
      const vertex_t v) const {
    return AdjList<typename boost_graph_t::out_edge_iterator>(
        boost::out_edges(v, *graph_));
  }

  AdjList<typename boost_graph_t::in_edge_iterator> GetIncomingAdjList(
      const vertex_t v) const {
    return AdjList<typename boost_graph_t::in_edge_iterator>(
        boost::in_edges(v, *graph_));
  }

  vertex_t source(const edge_t& e) const { return boost::source(e, *graph_); }

  vertex_t target(const edge_t& e) const { return boost::target(e, *graph_); }

  const edata_t& operator[](const edge_t& e) const {
    return graph_->operator[](e);
  }

  edata_t& operator[](const edge_t& e) { return graph_->operator[](e); }

  typename boost::graph_traits<boost_graph_t>::degree_size_type OutDegree(
      vertex_t v) const {
    return boost::out_degree(v, *graph_);
  }

  typename boost::graph_traits<boost_graph_t>::degree_size_type InDegree(
      vertex_t v) const {
    return boost::in_degree(v, *graph_);
  }

  bool edge(vertex_t u, vertex_t v, edge_t& edge) const {
    auto pair = boost::edge(u, v, *graph_);

    edge = pair.first;
    return pair.second;
  }

  std::shared_ptr<vertex_map_t> vertex_map() { return vertex_map_; }

 private:
  std::shared_ptr<vertex_map_t> vertex_map_;
  std::shared_ptr<boost_graph_t> graph_;
};

using EmptyType = boost::no_property;

}  // namespace her
#endif  // HER_GRAPH_H_
