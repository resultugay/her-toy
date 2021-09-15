#include "flags.h"

#include <gflags/gflags.h>

namespace her {
DEFINE_string(gd_efile, "", "edge file of graph GD");
DEFINE_string(gd_vfile, "", "vertex file of graph GD");
DEFINE_string(g_efile, "", "edge file of graph G");
DEFINE_string(g_vfile, "", "vertex file of graph G");
DEFINE_string(synonym_file, "", "a file contains synonym and score");
DEFINE_string(embedding_file, "", "pre-trained word embedding file");
DEFINE_string(gd_slabel_file, "", "A file contains source labels");
DEFINE_string(g_slabel_file, "", "A file contains source labels");
DEFINE_string(out_prefix, "", "output prefix");
DEFINE_int32(parallelism, -1, "How many threads will be used (only for apair)");
DEFINE_int32(bfs_depth, 3, "the depth of BFS used by function h_r");
DEFINE_string(desc_file, "", "A file contains vertex descendants of G");
DEFINE_string(path_file, "", "A file contains labels between v1 and v2 of G");
DEFINE_string(vpair_sources_file, "", "A file contains starting ids of gd");
DEFINE_int32(
    n_iter, 1,
    "Repeat -n_iter rounds evaluation to get a reliable timing result");

DEFINE_double(sigma, 0.8, "sigma threshold for vertex matching");
DEFINE_double(delta, 0.9, "delta threshold for path matching");
DEFINE_int32(k, 999999, "Top k descendants");

DEFINE_string(
    query_type, "",
    "query type: spair, spair_benchmark, vpair, vpair_benchmark, apair");

DEFINE_int32(vertex_u, 0, "vertex u of graph GD");
DEFINE_int32(vertex_v, 0, "vertex v of graph G");
}  // namespace her
