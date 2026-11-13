
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>

#include "clippy/keypath.hpp"
#include "examplegraph.hpp"

static const std::string method_name = "count";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name,
                      "returns a map containing the count of values in a "
                      "series based on keypath"};
  clip.add_required<keypath>("keypath",
                              "Existing keypath name to calculate extrema");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }
  auto kp_str = clip.get<keypath>("keypath");
  keypath kp{kp_str};

  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  bool is_edge_kp = examplegraph::examplegraph::is_edge_keypath(kp);
  bool is_node_kp = examplegraph::examplegraph::is_node_keypath(kp);

  if (!is_edge_kp && !is_node_kp) {
    std::cerr << "Keypath must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  auto tailkp_opt = kp.tail();
  if (!tailkp_opt) {
    std::cerr << "no tail" << std::endl;
    return 1;
  }

  auto tail_kp = tailkp_opt.value();
  if (is_edge_kp) {
    if (the_graph.has_series<bool>(kp)) {
      auto series = the_graph.get_edge_series<bool>(tail_kp);
      if (series) {
        clip.to_return<std::map<bool, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<bool, size_t>>({});
      }
    } else if (the_graph.has_series<int64_t>(kp)) {
      auto series = the_graph.get_edge_series<int64_t>(tail_kp);
      if (series) {
        clip.to_return<std::map<int64_t, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<int64_t, size_t>>({});
      }
    } else if (the_graph.has_series<double>(kp)) {
      auto series = the_graph.get_edge_series<double>(tail_kp);
      if (series) {
        clip.to_return<std::map<double, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<double, size_t>>({});
      }
    } else if (the_graph.has_series<std::string>(kp)) {
      auto series = the_graph.get_edge_series<std::string>(tail_kp);
      if (series) {
        clip.to_return<std::map<std::string, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<std::string, size_t>>({});
      }
    } else {
      std::cerr << "UNKNOWN TYPE" << std::endl;
      return 1;
    }
  } else if (is_node_kp) {
    if (the_graph.has_series<bool>(kp)) {
      kp = tailkp_opt.value();
      auto series = the_graph.get_node_series<bool>(tail_kp);
      if (series) {
        clip.to_return<std::map<bool, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<bool, size_t>>({});
      }
    } else if (the_graph.has_series<int64_t>(kp)) {
      auto series = the_graph.get_node_series<int64_t>(tail_kp);
      if (series) {
        clip.to_return<std::map<int64_t, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<int64_t, size_t>>({});
      }
    } else if (the_graph.has_series<double>(kp)) {
      auto series = the_graph.get_node_series<double>(tail_kp);
      if (series) {
        clip.to_return<std::map<double, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<double, size_t>>({});
      }
    } else if (the_graph.has_series<std::string>(kp)) {
      auto series = the_graph.get_node_series<std::string>(tail_kp);
      if (series) {
        clip.to_return<std::map<std::string, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<std::string, size_t>>({});
      }
    } else {
      std::cerr << "UNKNOWN TYPE" << std::endl;
      return 1;
    }
  }
  clip.set_state(state_name, the_graph);
  return 0;
}
