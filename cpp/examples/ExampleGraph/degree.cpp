
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <map>

#include "clippy/selector.hpp"
#include "examplegraph.hpp"

static const std::string method_name = "degree";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{
      method_name,
      "Populates a column containing the degree of each node in a graph"};
  clip.add_required<selector>(
      "selector",
      "Existing selector name into which the degree will be written");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");
  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for pending selectors");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  selector sel = clip.get<selector>("selector");

  if (!sel.headeq("node")) {
    std::cerr << "Selector must be a node subselector" << std::endl;
    return 1;
  }
  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  auto selectors =
      clip.get_state<std::map<std::string, std::string>>(sel_state_name);
  if (!selectors.contains(sel)) {
    std::cerr << "Selector not found" << std::endl;
    return 1;
  }
  auto subsel = sel.tail().value();
  if (the_graph.has_node_series(subsel)) {
    std::cerr << "Selector already populated" << std::endl;
    return 1;
  }

  auto deg_o = the_graph.add_node_series<int64_t>(subsel, "Degree");
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
  clip.set_state(sel_state_name, selectors);
  clip.update_selectors(selectors);

  clip.return_self();
  return 0;
}
