// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "testgraph.hpp"
#include <boost/json.hpp>

namespace boostjsn = boost::json;

static const std::string method_name = "ne";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Returns the number of edges in the graph"};

  clip.returns<size_t>("Number of edges.");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);

  clip.to_return<size_t>(the_graph.ne());
  return 0;
}
