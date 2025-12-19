
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>

#include "clippy/selector.hpp"
#include "examplegraph.hpp"

static const std::string method_name = "series_str";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name,
                      "returns the values of a series based on selector"};
  clip.add_required<selector>("selector", "Existing selector name");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");

  clip.returns<std::string>("String of data.");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }
  auto sel_str = clip.get<selector>("selector");
  selector sel{sel_str};

  bool is_edge_sel = examplegraph::examplegraph::is_edge_selector(sel);
  bool is_node_sel = examplegraph::examplegraph::is_node_selector(sel);

  if (!is_edge_sel && !is_node_sel) {
    std::cerr << "Selector must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  auto tail_opt = sel.tail();
  if (!tail_opt) {
    std::cerr << "Selector must have a tail" << std::endl;
    return 1;
  }
  auto tail_sel = tail_opt.value();

  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  if (is_edge_sel) {
    clip.to_return(the_graph.str_edge_col(tail_sel));
  } else if (is_node_sel) {
    clip.to_return(the_graph.str_node_col(tail_sel));
  } else {
    std::cerr << "Selector must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  //   clip.set_state(state_name, the_graph);
  return 0;
}
