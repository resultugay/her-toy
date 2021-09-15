#!/bin/bash

BIN_PREFIX=../build/her
GD_PREFIX=./dblp_small/gd
G_PREFIX=./dblp_small/g

function SPair() {
  ../build/her -gd_efile ./dblp_small/gd.e \
               -gd_vfile ./dblp_small/gd.v \
               -gd_slabel_file ./dblp_small/gd_slabels.txt \
               -g_efile ./dblp_small/g.e \
               -g_vfile ./dblp_small/g.v \
               -g_slabel_file ./dblp_small/g_slabels.txt \
               -synonym_file ./dblp_small/synonym.txt \
               -embedding_file ./dblp_small/glove.6B.50d.txt \
               -bfs_depth 4 \
               -query_type spair \
               -vertex_u 1188421 -vertex_v 1478400
}

function VPair() {
  ../build/her -gd_efile ./dblp_small/gd.e \
               -gd_vfile ./dblp_small/gd.v \
               -gd_slabel_file ./dblp_small/gd_slabels.txt \
               -g_efile ./dblp_small/g.e \
               -g_vfile ./dblp_small/g.v \
               -g_slabel_file ./dblp_small/g_slabels.txt \
               -synonym_file ./dblp_small/synonym.txt \
               -embedding_file ./dblp_small/glove.6B.50d.txt \
               -bfs_depth 4 \
               -query_type vpair \
               -vertex_u 1188421
}

function APair() {
  mpirun -n 12 ../build/her -gd_efile ./dblp_small/gd.e \
               -gd_vfile ./dblp_small/gd.v \
               -gd_slabel_file ./dblp_small/gd_slabels.txt \
               -g_efile ./dblp_small/g.e \
               -g_vfile ./dblp_small/g.v \
               -g_slabel_file ./dblp_small/g_slabels.txt \
               -synonym_file ./dblp_small/synonym.txt \
               -embedding_file ./dblp_small/glove.6B.50d.txt \
               -bfs_depth 4 \
               -out_prefix ./ \
               -query_type apair
}

echo "==============================================SPair=============================================="
SPair
echo "==============================================VPair=============================================="
VPair
echo "==============================================APair=============================================="
APair

