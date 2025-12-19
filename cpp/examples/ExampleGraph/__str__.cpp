// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "examplegraph.hpp"
#include <boost/json.hpp>
#include <iostream>

namespace boostjsn = boost::json;

static const std::string method_name = "__str__";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Str method for ExampleGraph"};

  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");

  clip.returns<std::string>("String of data.");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);
  clip.set_state(state_name, the_graph);

  std::stringstream sstr;
  sstr << "Graph with " << the_graph.nv() << " nodes and " << the_graph.ne()
       << " edges";

  clip.to_return(sstr.str());

  return 0;
}
