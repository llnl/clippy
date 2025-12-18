
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <variant>

#include "clippy/selector.hpp"
#include "testgraph.hpp"

static const std::string method_name = "count";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name,
                      "returns a map containing the count of values in a "
                      "series based on selector"};
  clip.add_required<selector>("selector",
                              "Existing selector name to calculate extrema");
  clip.add_required_state<testgraph::testgraph>(state_name,
                                                "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }
  auto sel_str = clip.get<selector>("selector");
  selector sel{sel_str};

  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);

  bool is_edge_sel = testgraph::testgraph::is_edge_selector(sel);
  bool is_node_sel = testgraph::testgraph::is_node_selector(sel);

  if (!is_edge_sel && !is_node_sel) {
    std::cerr << "Selector must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  auto tailsel_opt = sel.tail();
  if (!tailsel_opt) {
    std::cerr << "no tail" << std::endl;
    return 1;
  }

  auto tail_sel = tailsel_opt.value();
  if (is_edge_sel) {
    if (the_graph.has_series<bool>(sel)) {
      auto series = the_graph.get_edge_series<bool>(tail_sel);
      if (series) {
        clip.to_return<std::map<bool, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<bool, size_t>>({});
      }
    } else if (the_graph.has_series<int64_t>(sel)) {
      auto series = the_graph.get_edge_series<int64_t>(tail_sel);
      if (series) {
        clip.to_return<std::map<int64_t, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<int64_t, size_t>>({});
      }
    } else if (the_graph.has_series<double>(sel)) {
      auto series = the_graph.get_edge_series<double>(tail_sel);
      if (series) {
        clip.to_return<std::map<double, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<double, size_t>>({});
      }
    } else if (the_graph.has_series<std::string>(sel)) {
      auto series = the_graph.get_edge_series<std::string>(tail_sel);
      if (series) {
        clip.to_return<std::map<std::string, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<std::string, size_t>>({});
      }
    } else {
      std::cerr << "UNKNOWN TYPE" << std::endl;
      return 1;
    }
  } else if (is_node_sel) {
    if (the_graph.has_series<bool>(sel)) {
      sel = tailsel_opt.value();
      auto series = the_graph.get_node_series<bool>(tail_sel);
      if (series) {
        clip.to_return<std::map<bool, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<bool, size_t>>({});
      }
    } else if (the_graph.has_series<int64_t>(sel)) {
      auto series = the_graph.get_node_series<int64_t>(tail_sel);
      if (series) {
        clip.to_return<std::map<int64_t, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<int64_t, size_t>>({});
      }
    } else if (the_graph.has_series<double>(sel)) {
      auto series = the_graph.get_node_series<double>(tail_sel);
      if (series) {
        clip.to_return<std::map<double, size_t>>(series.value().count());
      } else {
        clip.to_return<std::map<double, size_t>>({});
      }
    } else if (the_graph.has_series<std::string>(sel)) {
      auto series = the_graph.get_node_series<std::string>(tail_sel);
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
