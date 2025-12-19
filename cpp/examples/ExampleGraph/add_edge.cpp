// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "examplegraph.hpp"
#include <boost/json.hpp>

namespace boostjsn = boost::json;

static const std::string method_name = "add_edge";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Inserts (src, dst) into an ExampleGraph"};
  clip.add_required<std::string>("src", "source node");
  clip.add_required<std::string>("dst", "dest node");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");
  clip.returns_self();

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto src = clip.get<std::string>("src");
  auto dst = clip.get<std::string>("dst");
  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);
  the_graph.add_edge(src, dst);
  clip.set_state(state_name, the_graph);
  clip.return_self();
  return 0;
}
