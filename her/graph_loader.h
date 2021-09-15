#ifndef HER_GRAPH_LOADER_H_
#define HER_GRAPH_LOADER_H_
#include <glog/logging.h>

#include <boost/algorithm/string.hpp>
#include <boost/mpi.hpp>
#include <fstream>
#include <vector>

#include "her/graph.h"

namespace her {
template <typename GRAPH_T>
class GraphLoader {
 private:
  using oid_t = typename GRAPH_T::oid_t;
  using vid_t = typename GRAPH_T::vid_t;
  using vdata_t = typename GRAPH_T::vdata_t;
  using edata_t = typename GRAPH_T::edata_t;
  using boost_graph_t = typename GRAPH_T::boost_graph_t;

 public:
  GRAPH_T LoadGraph(const std::string& vfile, const std::string& efile) {
    auto vm_ptr = std::make_shared<VertexMap<oid_t, vid_t>>();
    std::vector<vdata_t> vertex_data;
    std::vector<std::pair<vid_t, vid_t>> edges;
    std::vector<edata_t> edge_data;
    boost::mpi::communicator comm;

    {
      std::ifstream fi(vfile);
      std::string line;
      size_t line_no = 0;

      while (std::getline(fi, line)) {
        oid_t oid;
        vid_t lid;
        vdata_t data;
        ++line_no;
        if (line_no % 1000000 == 0) {
          VLOG(10) << "Read " << line_no << " lines";
        }
        if (line.empty() || line[0] == '#')
          continue;

        std::istringstream ss(line);
        ss >> oid;
        ss.ignore(1);  // whitespace
        std::getline(ss, data);

        CHECK(vm_ptr->AddVertex(oid, lid))
            << "Duplicate vertex: " << oid << " line no: " << line_no;
        boost::trim(data);
        vertex_data.push_back(data);
      }
      fi.close();
      LOG(INFO) << "Rank: " << comm.rank() << " Loaded " << vfile << ": "
                << vm_ptr->TotalVertexNum() << " vertices.";
    }

    {
      std::ifstream fi(efile);
      std::string line;
      size_t line_no = 0;

      while (std::getline(fi, line)) {
        oid_t src_oid, dst_oid;
        vid_t src_lid, dst_lid;
        edata_t data;
        ++line_no;
        if (line_no % 1000000 == 0) {
          VLOG(10) << "Read " << line_no << " lines";
        }
        if (line.empty() || line[0] == '#')
          continue;

        std::istringstream ss(line);

        ss >> src_oid;
        ss >> dst_oid;
        ss.ignore(1);  // whitespace
        std::getline(ss, data);

        CHECK(vm_ptr->GetLid(src_oid, src_lid))
            << "Missing src vertex " << src_oid
            << ". Failed to process edge: " << line;
        CHECK(vm_ptr->GetLid(dst_oid, dst_lid))
            << "Missing dst vertex " << dst_oid
            << ". Failed to process edge: " << line;
        edges.template emplace_back(src_lid, dst_lid);
        boost::trim(data);
        edge_data.template emplace_back(data);
      }
      fi.close();
      LOG(INFO) << "Rank: " << comm.rank() << " Loaded " << efile << ": "
                << edges.size() << " edges.";
    }

    std::size_t nvnum = vm_ptr->TotalVertexNum();
    auto graph_ptr = std::make_shared<boost_graph_t>(
        boost::edges_are_unsorted_multi_pass, edges.begin(), edges.end(),
        edge_data.begin(), nvnum);
    auto& g = *graph_ptr;

    int index = 0;

    std::pair<typename boost::graph_traits<boost_graph_t>::vertex_iterator,
              typename boost::graph_traits<boost_graph_t>::vertex_iterator>
        vertices = boost::vertices(g);

    for (auto it = vertices.first; it != vertices.second; it++) {
      typename boost::graph_traits<boost_graph_t>::vertex_descriptor vd = *it;

      g[vd] = vertex_data[index++];
    }

    return {vm_ptr, graph_ptr};
  }
};

}  // namespace her

#endif  // HER_GRAPH_LOADER_H_
