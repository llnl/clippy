// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <cassert>
#include <iostream>

#include "clippy/clippy.hpp"
#include "clippy/selector.hpp"
#include "testgraph.hpp"

namespace boostjsn = boost::json;

static const std::string method_name = "add_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Adds a subselector"};
  clip.add_required<selector>("parent_sel", "Parent Selector");
  clip.add_required<std::string>("sub_sel", "Name of new selector");
  clip.add_optional<std::string>("desc", "Description of new selector", "");

  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for pending selectors");
  clip.add_required_state<testgraph::testgraph>(graph_state_name,
                                                "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  std::string parstr = clip.get<selector>("parent_sel");
  auto parsel = selector{parstr};
  auto subsel = clip.get<std::string>("sub_sel");
  auto desc = clip.get<std::string>("desc");

  std::string fullname = parstr + "." + subsel;

  // std::map<std::string, std::string> selectors;
  auto the_graph = clip.get_state<testgraph::testgraph>(graph_state_name);

  if (parsel.headeq("edge")) {
    if (the_graph.has_edge_series(subsel)) {
      std::cerr << "!! ERROR: Selector name already exists in edge table !!"
                << std::endl;
      exit(-1);
    }
  } else if (parsel.headeq("node")) {
    if (the_graph.has_node_series(subsel)) {
      std::cerr << "!! ERROR: Selector name already exists in node table !!"
                << std::endl;
      exit(-1);
    }
  } else {
    std::cerr
        << "((!! ERROR: Parent must be either \"edge\" or \"node\" (received "
        << parstr << ") !!)";
    exit(-1);
  }

  if (clip.has_state(sel_state_name)) {
    auto selectors =
        clip.get_state<std::map<std::string, std::string>>(sel_state_name);
    if (selectors.contains(fullname)) {
      std::cerr << "Warning: Selector name already exists; ignoring"
                << std::endl;
    } else {
      selectors[fullname] = desc;
      clip.set_state(sel_state_name, selectors);
      clip.update_selectors(selectors);
      clip.return_self();
    }
  }

  return 0;
}
