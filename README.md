# Parametric Simulation Program

This folder contains all necessary source code and scripts to demonstrate the parametric simulation algorithm in the paper "Linking Entities across Relations and Graphs". 
In addition, a dataset and script are also
provided to exemplify how to run the program. 

## 1. Prerequisites

- Building tools: gcc/clang, cmake
- Dependencies: gflags, glog, boost >= 1.65, OpenMPI

## 2. Build

For building the program, just type in `cmake ..` and `build`

```bash
mkdir build && cd build
cmake .. && make
```

then, a binary named `her` is ready to use. There are a variety of parameters 
to allow us to use and tune the program.

## 3. Data preparation

### 3.1 Dataset
The program requires two datasets that are `GD` and `G`.
The options `-gd_efile`, `-gd_vfile`, `-g_efile` and `-g_vfile` allow you to pass paths of datasets to the program. 
These options specify the location of files that contain the edges and vertices of graphs GD and G, respectively.
Each line in the vertex file describes the vertex id and label, and there is no limitation about the vertex id, as long as it is a number.
The dataset is large and the toy example is in the following link
https://www.dropbox.com/sh/f7xcm353nz76kf0/AABygzfrN5YwLT5IgyTabgJIa?dl=0

For example, first 10 lines of `g.v`
```
0 conf/stoc/Dietzfelbinger97
1 people-848e039ab1ee1e4d29a6aabe3bd8a438-031b0bd04939992cda46161d83d04bd5
2 people-00fc5b3df452baa1c28cc8bd0b5a53c7-ed12578a98034431fdf89dc3c67101c1
3 Andrew Lim
4 journals/cn/GallH97
5 journals-785b09432d03b41dd88652d31b501d58
6 conf/fct/Calabrese97
7 http://dblp.l3s.de/d2r/resource/publications/conf/fct/Calabrese97
8 conf/icip/LeeCZ97
9 people-d89c8625e6c79fa10fff30683e5fa125-47ae500f9f6230b55618c832f2edb94d
```

In terms of the format of the edge file, every line in the edge file consists of three parts: the id of source vertex, destination vertex, and an edge label.
One thing should be noticed that **the vertex id (no matter it is source or destination) should be given in the vertex file**.
Otherwise, an error will be raised by the program.

And first 10 lines of `g.e`,
```
0 1 has-author
0 25 has-date
0 98316 has-title
0 114591 sameAs
0 17 type
0 149772 has-web-address
0 949 article-of-journal
0 6172 cites-publication-reference
0 76 type
0 9093353 has-date
```


### 3.2 Word embedding

We use pre-trained word vector, which comes from https://nlp.stanford.edu/projects/glove/. 
So the preparation is quite straightforward, just download and unzip it, then just feed the 
pre-trained word vector to the program with an option `-embedding_file`.

### 3.3 Entity labels
The query time can be reduced up by filtering. If the query only starts from a pair of vertices that represents entities, 
a lot of redundant matching can be avoided. 
Thus, the program expects a pair of files that contain the entity labels of GD and G. 

These two files can be easily made by extracting the entity labels of G/GD files.
Once these files are ready, specifying the paths of these with `gd_slabel_file` and `g_slabel_file` options.

N.B. These files are only required for the APair query.

## 4. Initiate a query
The following examples illustrate how to use SPair, VPair, and APair algorithms.

### SPair query
For launching a SPair query, the program demands the option `-vertex_u`, which represents a vertex of graph GD, and a vertex of G should be denoted with `-vertex_v`.
For instance,
```bash
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
```

### VPair query

For VPair query, instead of requiring a pair of vertices, 
the program only needs vertex u, which is an entity vertex of graph GD.
```bash
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
```

### APair query
For initiating an APair query, you should pass an option `-query_type apair` to the program. 
Typical, an APair query will take thousands of seconds, but we can reduce the time to hundreds or even lower with parallel techniques. 

Just append your query command after `mpirun`, like `mpirun -hostfile machine-list ./her [QUERY PARAMETERS]`,
then tasks spawned by the query will be distributed among the workers, and computation begins.

```bash
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
```

## 5.Examples
We provided a made-ready dataset under the `test/dblp_small` folder,
and a script `run.sh` which allows you to run all three types of query. 
Before the execution, you should these the binary `her` is build under the folder
of `/build/her`

```
her@iZ0jl3j3w46zstlmojyf9vZ:/tmp/tmp.zK3avdeQn4/build$ cd ..
her@iZ0jl3j3w46zstlmojyf9vZ:/tmp/tmp.zK3avdeQn4$ rm -rf build/
her@iZ0jl3j3w46zstlmojyf9vZ:/tmp/tmp.zK3avdeQn4$ mkdir build
her@iZ0jl3j3w46zstlmojyf9vZ:/tmp/tmp.zK3avdeQn4$ cd build/
her@iZ0jl3j3w46zstlmojyf9vZ:/tmp/tmp.zK3avdeQn4/build$ cmake ..
-- The C compiler identification is GNU 7.5.0
-- The CXX compiler identification is GNU 7.5.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found MPI_C: /usr/lib/x86_64-linux-gnu/openmpi/lib/libmpi.so (found version "3.1")
-- Found MPI_CXX: /usr/lib/x86_64-linux-gnu/openmpi/lib/libmpi_cxx.so (found version "3.1")
-- Found MPI: TRUE (found version "3.1")
-- Boost version: 1.65.1
-- Found the following Boost libraries:
--   serialization
--   mpi
--   graph
-- Using boost.
-- Boost_INCLUDE_DIRS: /usr/include
-- Boost_LIBRARIES: /usr/lib/x86_64-linux-gnu/libboost_serialization.so;/usr/lib/x86_64-linux-gnu/libboost_mpi.so;/usr/lib/x86_64-linux-gnu/libboost_graph.so
-- Boost_VERSION: 106501
-- Found Glog: /usr/include
-- Found glog    (include: /usr/include, library: /usr/lib/x86_64-linux-gnu/libglog.so)
-- Found GFlags: /usr/include
-- Found gflags  (include: /usr/include, library: /usr/lib/x86_64-linux-gnu/libgflags.so)
-- Configuring done
-- Generating done
-- Build files have been written to: /tmp/tmp.zK3avdeQn4/build
her@iZ0jl3j3w46zstlmojyf9vZ:/tmp/tmp.zK3avdeQn4/build$ make
Scanning dependencies of target her
[ 33%] Building CXX object CMakeFiles/her.dir/her/her.cc.o
[ 66%] Building CXX object CMakeFiles/her.dir/her/flags.cc.o
[100%] Linking CXX executable her
[100%] Built target her
```

And then, enter the `test` folder, simply exeucte `run.sh` without any extra command.

```
her@iZ0jl3j3w46zstlmojyf9vZ:/tmp/tmp.zK3avdeQn4/test$ ./run.sh
==============================================SPair==============================================
I0713 16:00:57.087599  9638 her.h:391] Rank: 0 thread num: 24
I0713 16:00:57.087939  9638 her.h:193] Loaded ./dblp_small/synonym.txt: 16 words
I0713 16:01:00.649587  9646 graph_loader.h:57] Rank: 0 Loaded ./dblp_small/gd.v: 1403905 vertices.
I0713 16:01:01.288295  9647 graph_loader.h:57] Rank: 0 Loaded ./dblp_small/g.v: 1659259 vertices.
I0713 16:01:05.886905  9646 graph_loader.h:95] Rank: 0 Loaded ./dblp_small/gd.e: 2232304 edges.
I0713 16:01:07.705090  9648 her.h:158] Rank: 0 Loaded ./dblp_small/glove.6B.50d.txt: 400000 words
I0713 16:01:08.313508  9647 graph_loader.h:95] Rank: 0 Loaded ./dblp_small/g.e: 2938531 edges.
I0713 16:01:24.087764  9638 spair.h:106] Max delta: 1.6 Avg delta: 0.88
I0713 16:01:24.087824  9638 her.h:518] Query: (1188421, 1478400) = True
Timing results:
 - Load Data: 19.1035 sec

 - Filling word vector: 3.63201 sec

 - Init inverted index: 4.26355 sec

 - Query: 0.00102592 sec

==============================================VPair==============================================
I0713 16:01:25.810081  9697 her.h:391] Rank: 0 thread num: 24
I0713 16:01:25.810425  9697 her.h:193] Loaded ./dblp_small/synonym.txt: 16 words
I0713 16:01:29.410516  9705 graph_loader.h:57] Rank: 0 Loaded ./dblp_small/gd.v: 1403905 vertices.
I0713 16:01:30.039942  9706 graph_loader.h:57] Rank: 0 Loaded ./dblp_small/g.v: 1659259 vertices.
I0713 16:01:34.678171  9705 graph_loader.h:95] Rank: 0 Loaded ./dblp_small/gd.e: 2232304 edges.
I0713 16:01:36.513612  9707 her.h:158] Rank: 0 Loaded ./dblp_small/glove.6B.50d.txt: 400000 words
I0713 16:01:37.062208  9706 graph_loader.h:95] Rank: 0 Loaded ./dblp_small/g.e: 2938531 edges.
I0713 16:01:55.182354  9697 vpair.h:30] Calculate candidate: 2.37756 s C size: 153094
I0713 16:02:12.257930  9697 vpair.h:54] SPair query: 16.9369 s
I0713 16:02:12.257982  9697 spair.h:106] Max delta: 1.6 Avg delta: 5.73391e-05
I0713 16:02:12.509363  9697 her.h:551] Matched pairs:
I0713 16:02:12.509393  9697 her.h:557] 1478400|brief encounters with a random key graph.
Timing results:
 - Load Data: 19.143 sec

 - Filling word vector: 3.56921 sec

 - Init inverted index: 4.2824 sec

 - Query: 19.7045 sec

 - Output: 5.91278e-05 sec

==============================================APair==============================================
I0713 16:02:14.290607  9762 her.h:391] Rank: 1 thread num: 2
I0713 16:02:14.290609  9763 her.h:391] Rank: 2 thread num: 2
......
I0713 16:04:34.123376  9763 apair_parallel.h:142] Rank: 2 Generate candidates: 73.1618 s, local candidates: 15415
I0713 16:04:36.651840  9774 apair_parallel.h:142] Rank: 7 Generate candidates: 75.6903 s, local candidates: 15407
......
I0713 16:04:54.555728  9784 apair_parallel.h:180] Rank: 11 |pairs|: 26810 SPair query: 14.253 s
I0713 16:04:54.558203  9784 spair.h:106] Max delta: 1.6 Avg delta: 0.692363
I0713 16:04:55.312319  9763 apair_parallel.h:180] Rank: 2 |pairs|: 31415 SPair query: 15.0096 s
I0713 16:04:55.314836  9763 spair.h:106] Max delta: 1.6 Avg delta: 0.653191
I0713 16:04:58.206346  9767 apair_parallel.h:180] Rank: 5 |pairs|: 35072 SPair query: 17.9037 s
......
Timing results:
 - Load Data: 32.0379 sec

 - Filling word vector: 28.9365 sec

 - Init inverted index: 5.69613 sec

 - Query: 194.8 sec

 - Average Query: 194.8 sec

 - Output: 0.000184059 sec

```


## 6.Misc
For performance evaluation, we use a BFS search to look up all the descendants of a vertex. 
The option `-bfs_depth` can be used to limit the depth of the search, but we also allow users to 
provide descendants of vertices by the option `-desc_file`.

The option `-path_file` can be used to specify the edge label of vertices of v1 and v2 in graph G.
If this option is omitted, the program will brutally search all the potential paths.

When the option `n_iter` is given, the algorithm will be evaluated multiple times, and the average running time will be reported.

Query result of SPair and VPair will be printed directly to the console. For APair, the result will be written into local file, 
and the output path can be give by `-out_prefix`.