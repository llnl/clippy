// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "testgraph.hpp"
#include <boost/json.hpp>

namespace boostjsn = boost::json;

static const std::string method_name = "add_node";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Inserts a node into a TestGraph"};
  clip.add_required<std::string>("node", "node to insert");
  clip.add_required_state<testgraph::testgraph>(state_name,
                                                "Internal container");
  clip.returns_self();

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto node = clip.get<std::string>("node");
  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);
  the_graph.add_node(node);
  clip.set_state(state_name, the_graph);
  clip.return_self();
  return 0;
}
