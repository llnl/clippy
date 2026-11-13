
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <cassert>
#include <iostream>

#include "clippy/clippy.hpp"
#include "clippy/keypath.hpp"
#include "examplegraph.hpp"

namespace boostjsn = boost::json;

static const std::string method_name = "drop_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string kp_state_name = "keypaths";
static const std::string kp_name = "keypath";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Drops a keypath"};
  clip.add_required<keypath>(kp_name, "Keypath to drop");

  clip.add_required_state<std::map<std::string, std::string>>(
      kp_state_name, "Internal container for pending keypaths");
  clip.add_required_state<examplegraph::examplegraph>(
      graph_state_name, "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto jo = clip.get<boostjsn::object>(kp_name);

  keypath kp = clip.get<keypath>(kp_name);

  auto kp_state =
      clip.get_state<std::map<std::string, std::string>>(kp_state_name);
  if (!kp_state.contains(kp)) {
    std::cerr << "Keypath name not found!" << std::endl;
    exit(-1);
  }
  auto the_graph = clip.get_state<examplegraph::examplegraph>(graph_state_name);
  auto subpath = kp.tail().value();
  if (kp.headeq("edge")) {
    if (the_graph.has_edge_series(subpath)) {
      the_graph.drop_edge_series(kp);
    }
  } else if (kp.headeq("node")) {
    if (the_graph.has_node_series(subpath)) {
      the_graph.drop_node_series(subpath);
    }
  } else {
    std::cerr << "Keypath name must start with either \"edge.\" or \"node.\""
              << std::endl;
    exit(-1);
  }
  kp_state.erase(kp);
  clip.set_state(graph_state_name, the_graph);
  clip.set_state(kp_state_name, kp_state);
  clip.update_keypaths(kp_state);

  return 0;
}
