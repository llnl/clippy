#pragma once
#include "mvmap.hpp"
#include <cstdint>
#include <map>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace examplegraph {
// map of (src, dst) : weight
using node_t = std::string;
using edge_t = std::pair<node_t, node_t>;

template <typename T> using sparsevec = std::map<uint64_t, T>;

enum series_type { ser_bool, ser_int64, ser_double, ser_string, ser_invalid };

using variants = std::variant<bool, double, int64_t, std::string>;
class examplegraph {
  using edge_mvmap = mvmap::mvmap<edge_t, bool, int64_t, double, std::string>;
  using node_mvmap = mvmap::mvmap<node_t, bool, int64_t, double, std::string>;
  template <typename T> using edge_series_proxy = edge_mvmap::series_proxy<T>;
  template <typename T> using node_series_proxy = node_mvmap::series_proxy<T>;
  node_mvmap node_table;
  edge_mvmap edge_table;

public:
  const mvmap::mvmap<node_t, bool, int64_t, double, std::string> &
  nodemap() const {
    return node_table;
  }

  const mvmap::mvmap<edge_t, bool, int64_t, double, std::string> &
  edgemap() const {
    return edge_table;
  }
  static inline bool is_edge_selector(const std::string &sel) {
    return sel.starts_with("edge.");
  }

  static inline bool is_node_selector(const std::string &sel) {
    return sel.starts_with("node.");
  }

  static inline bool is_valid_selector(const std::string &sel) {
    return is_edge_selector(sel) || is_node_selector(sel);
  }

  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, examplegraph const &g) {
    v = {{"node_table", boost::json::value_from(g.node_table)},
         {"edge_table", boost::json::value_from(g.edge_table)}};
  }

  friend examplegraph
  tag_invoke(boost::json::value_to_tag<examplegraph> /*unused*/,
             const boost::json::value &v) {
    const auto &obj = v.as_object();
    auto nt = boost::json::value_to<node_mvmap>(obj.at("node_table"));
    auto et = boost::json::value_to<edge_mvmap>(obj.at("edge_table"));
    return {nt, et};
  }
  examplegraph() = default;
  examplegraph(node_mvmap nt, edge_mvmap et)
      : node_table(std::move(nt)), edge_table(std::move(et)) {};

  // this function requires that the "edge." prefix be removed from the name.
  template <typename T>
  std::optional<edge_series_proxy<T>>
  add_edge_series(const std::string &sel, const std::string &desc = "") {
    return edge_table.add_series<T>(sel, desc);
  }

  template <typename T>
  std::optional<edge_series_proxy<T>>
  add_edge_series(const std::string &sel, const edge_series_proxy<T> &from,
                  const std::string &desc = "") {
    return edge_table.add_series<T>(sel, from, desc);
  }

  void drop_edge_series(const std::string &sel) { edge_table.drop_series(sel); }

  // this function requires that the "node." prefix be removed from the name.
  void drop_node_series(const std::string &sel) { node_table.drop_series(sel); }

  // this function requires that the "node." prefix be removed from the name.
  template <typename T>
  std::optional<node_series_proxy<T>>
  add_node_series(const std::string &sel, const std::string &desc = "") {
    return node_table.add_series<T>(sel, desc);
  }

  template <typename T>
  std::optional<edge_series_proxy<T>>
  add_node_series(const std::string &sel, const node_series_proxy<T> &from,
                  const std::string &desc = "") {
    return node_table.add_series<T>(sel, from, desc);
  }
  template <typename T>
  std::optional<edge_series_proxy<T>> get_edge_series(const std::string &sel) {
    return edge_table.get_series<T>(sel);
  }

  bool copy_edge_series(const std::string &from, const std::string &to,
                        const std::optional<std::string> &desc = std::nullopt) {
    return edge_table.copy_series(from, to, desc);
  }

  bool copy_node_series(const std::string &from, const std::string &to,
                        const std::optional<std::string> &desc = std::nullopt) {
    return node_table.copy_series(from, to, desc);
  }

  template <typename T>
  std::optional<node_series_proxy<T>> get_node_series(const std::string &sel) {
    return node_table.get_series<T>(sel);
  }

  [[nodiscard]] size_t nv() const { return node_table.size(); }
  [[nodiscard]] size_t ne() const { return edge_table.size(); }

  template <typename F> void for_all_edges(F f) { edge_table.for_all(f); }
  template <typename F> void for_all_nodes(F f) { node_table.for_all(f); }

  [[nodiscard]] std::vector<edge_t> edges() const {
    auto kv = edge_table.keys();
    return {kv.begin(), kv.end()};
  }
  [[nodiscard]] std::vector<node_t> nodes() const {
    auto kv = node_table.keys();
    return {kv.begin(), kv.end()};
  }

  bool add_node(const node_t &node) { return node_table.add_key(node); };
  bool add_edge(const node_t &src, const node_t &dst) {
    node_table.add_key(src);
    node_table.add_key(dst);
    return edge_table.add_key({src, dst});
  }

  bool has_node(const node_t &node) { return node_table.contains(node); };
  bool has_edge(const edge_t &edge) { return edge_table.contains(edge); };
  bool has_edge(const node_t &src, const node_t &dst) {
    return edge_table.contains({src, dst});
  };

  // strips the head off the std::string and passes the tail to the appropriate
  // method.
  [[nodiscard]] bool has_series(const std::string &sel) const {
    auto tail = sel.substr(5);

    if (is_node_selector(sel)) {
      return has_node_series(tail);
    }
    if (is_edge_selector(sel)) {
      return has_edge_series(tail);
    }
    return false;
  }

  template <typename T>
  [[nodiscard]] bool has_series(const std::string &sel) const {
    auto tail = sel.substr(5);

    if (is_node_selector(sel)) {
      return has_node_series<T>(tail);
    }
    if (is_edge_selector(sel)) {
      return has_edge_series<T>(tail);
    }
    return false;
  }

  // assumes sel has already been tail'ed.
  [[nodiscard]] bool has_node_series(const std::string &sel) const {
    return node_table.has_series(sel);
  }

  template <typename T>
  [[nodiscard]] bool has_node_series(const std::string &sel) const {
    return node_table.has_series<T>(sel);
  }

  // assumes sel has already been tail'ed.
  [[nodiscard]] bool has_edge_series(const std::string &sel) const {
    return edge_table.has_series(sel);
  }
  template <typename T>
  [[nodiscard]] bool has_edge_series(const std::string &sel) const {
    return edge_table.has_series<T>(sel);
  }

  [[nodiscard]] std::vector<node_t> out_neighbors(const node_t &node) const {
    std::vector<node_t> neighbors;
    for (const auto &[src, dst] : edge_table.keys()) {
      if (src == node) {
        neighbors.emplace_back(dst);
      }
    }
    return neighbors;
  }

  [[nodiscard]] std::vector<node_t> in_neighbors(const node_t &node) const {
    std::vector<node_t> neighbors;
    for (const auto &[src, dst] : edge_table.keys()) {
      if (dst == node) {
        neighbors.emplace_back(src);
      }
    }
    return neighbors;
  }

  [[nodiscard]] size_t in_degree(const node_t &node) const {
    return in_neighbors(node).size();
  }
  [[nodiscard]] size_t out_degree(const node_t &node) const {
    return out_neighbors(node).size();
  }

  std::string str_edge_col(const std::string &col) {
    std::vector<std::string> cols{col};
    return edge_table.str_cols(cols);
  }

  std::string str_node_col(const std::string &col) {
    std::vector<std::string> cols{col};
    return node_table.str_cols(cols);
  }

}; // class examplegraph

} // namespace examplegraph
