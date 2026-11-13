
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <map>

#include "clippy/keypath.hpp"
#include "examplegraph.hpp"

static const std::string method_name = "degree";
static const std::string state_name = "INTERNAL";
static const std::string kp_state_name = "keypaths";

int main(int argc, char **argv) {
  clippy::clippy clip{
      method_name,
      "Populates a column containing the degree of each node in a graph"};
  clip.add_required<keypath>(
      "keypath",
      "Existing keypath name into which the degree will be written");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");
  clip.add_required_state<std::map<std::string, std::string>>(
      kp_state_name, "Internal container for pending keypaths");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  keypath kp = clip.get<keypath>("keypath");

  if (!kp.headeq("node")) {
    std::cerr << "Keypath must be a node subpath" << std::endl;
    return 1;
  }
  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  auto keypaths =
      clip.get_state<std::map<std::string, std::string>>(kp_state_name);
  if (!keypaths.contains(kp)) {
    std::cerr << "Keypath not found" << std::endl;
    return 1;
  }
  auto subpath = kp.tail().value();
  if (the_graph.has_node_series(subpath)) {
    std::cerr << "Keypath already populated" << std::endl;
    return 1;
  }

  auto deg_o = the_graph.add_node_series<int64_t>(subpath, "Degree");
  if (!deg_o) {
    std::cerr << "Unable to manifest node series" << std::endl;
    return 1;
  }

  auto deg = deg_o.value();

  the_graph.for_all_edges([&deg](auto edge, mvmap::locator /*unused*/) {
    deg[edge.first]++;
    if (edge.first != edge.second) {
      deg[edge.second]++;
    }
  });

  clip.set_state(state_name, the_graph);
  clip.set_state(kp_state_name, keypaths);
  clip.update_keypaths(keypaths);

  clip.return_self();
  return 0;
}
