// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "testgraph.hpp"
#include <boost/json.hpp>

namespace boostjsn = boost::json;

static const std::string method_name = "__init__";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Initializes a TestGraph"};

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  // testgraph needs to be convertible to json
  testgraph::testgraph the_graph;
  clip.set_state(state_name, the_graph);
  std::map<std::string, std::string> selectors;
  clip.set_state(sel_state_name, selectors);

  return 0;
}
