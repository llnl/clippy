
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

static const std::string method_name = "series_str";
static const std::string state_name = "INTERNAL";
static const std::string kp_state_name = "keypaths";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name,
                      "returns the values of a series based on keypath"};
  clip.add_required<keypath>("keypath", "Existing keypath name");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");

  clip.returns<std::string>("String of data.");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }
  auto kp_str = clip.get<keypath>("keypath");
  keypath kp{kp_str};

  bool is_edge_kp = examplegraph::examplegraph::is_edge_keypath(kp);
  bool is_node_kp = examplegraph::examplegraph::is_node_keypath(kp);

  if (!is_edge_kp && !is_node_kp) {
    std::cerr << "Keypath must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  auto tail_opt = kp.tail();
  if (!tail_opt) {
    std::cerr << "Keypath must have a tail" << std::endl;
    return 1;
  }
  auto tail_kp = tail_opt.value();

  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  if (is_edge_kp) {
    clip.to_return(the_graph.str_edge_col(tail_kp));
  } else if (is_node_kp) {
    clip.to_return(the_graph.str_node_col(tail_kp));
  } else {
    std::cerr << "Keypath must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  //   clip.set_state(state_name, the_graph);
  return 0;
}
