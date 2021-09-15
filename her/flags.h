
#ifndef HER_FLAGS_H_
#define HER_FLAGS_H_
#include <gflags/gflags_declare.h>
namespace her {
DECLARE_string(gd_efile);
DECLARE_string(gd_vfile);
DECLARE_string(g_efile);
DECLARE_string(g_vfile);
DECLARE_string(synonym_file);
DECLARE_string(embedding_file);
DECLARE_string(gd_slabel_file);
DECLARE_string(g_slabel_file);
DECLARE_string(out_prefix);
DECLARE_int32(parallelism);
DECLARE_string(index_type);
DECLARE_int32(bfs_depth);
DECLARE_string(desc_file);
DECLARE_string(path_file);
DECLARE_string(vpair_sources_file);
DECLARE_int32(n_iter);
DECLARE_bool(measure);

DECLARE_double(sigma);
DECLARE_double(delta);
DECLARE_int32(k);

DECLARE_string(query_type);

DECLARE_int32(vertex_u);
DECLARE_int32(vertex_v);
}  // namespace her
#endif  // HER_FLAGS_H_
