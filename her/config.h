#ifndef PARAMATRICSIMULATION_HER_CONFIG_H_
#define PARAMATRICSIMULATION_HER_CONFIG_H_

#include <boost/functional/hash.hpp>
#include <vector>
#include "Eigen/Eigen"

namespace her {
template <typename CoordinateT>
using dense_vector_t = Eigen::Matrix<CoordinateT, Eigen::Dynamic, 1, Eigen::ColMajor>;

using depth_t = uint16_t;
}  // namespace her

template <class T1, class T2>
struct std::hash<std::pair<T1, T2>> {
  std::size_t operator()(const std::pair<T1, T2>& pair) const {
    return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
  }
};

template <class T>
struct std::hash<her::dense_vector_t<T>> {
  std::size_t operator()(const her::dense_vector_t<T>& e) const {
    std::size_t seed = 0;

    for (int i = 0; i < e.size(); i++) {
      boost::hash_combine(seed, e[i]);
    }
    return seed;
  }
};
#endif  // PARAMATRICSIMULATION_HER_CONFIG_H_
