#ifndef PARAMATRICSIMULATION_HER_INVERTED_INDEX_H_
#define PARAMATRICSIMULATION_HER_INVERTED_INDEX_H_
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "boost/algorithm/string.hpp"

template <typename GRAPH_T>
class InvertedIndex {
  using vertex_t = typename GRAPH_T::vertex_t;
  std::unordered_set<std::string> blank_word = {"and", "or", "for",
                                                "in",  "on", "of"};

 public:
  void Init(const GRAPH_T& g,
            const std::unordered_set<std::string>& g_source_labels) {
    for (auto& v : g.Vertices()) {
      auto& label = g[v];

      if (g.OutDegree(v) > 0 &&
          g_source_labels.find(label) != g_source_labels.end()) {
        std::vector<std::string> words;

        boost::split(words, label, boost::is_any_of("\t "),
                     boost::token_compress_on);

        for (auto& word : words) {
          if (blank_word.find(word) == blank_word.end()) {
            word_indices_[word].insert(v);
          }
        }
      }
    }
  }

  std::set<vertex_t> Query(const std::string& label) const {
    std::vector<std::string> words;
    std::set<vertex_t> result;

    boost::split(words, label, boost::is_any_of("\t "),
                 boost::token_compress_on);

    for (auto& word : words) {
      auto it = word_indices_.find(word);

      if (it != word_indices_.end()) {
        auto& curr_set = it->second;

        if (result.empty()) {
          result = curr_set;
        } else {
          std::set<vertex_t> tmp;
          std::set_intersection(result.begin(), result.end(), curr_set.begin(),
                                curr_set.end(),
                                std::inserter(tmp, tmp.begin()));
          result = tmp;
          if (result.empty()) {
            return {};
          }
        }
      }
    }

    return result;
  }

 private:
  std::unordered_map<std::string, std::set<vertex_t>> word_indices_;
};
#endif  // PARAMATRICSIMULATION_HER_INVERTED_INDEX_H_
