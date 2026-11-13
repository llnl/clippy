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

static const std::string method_name = "add_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string kp_state_name = "keypaths";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Adds a subpath"};
  clip.add_required<keypath>("parent_keypath", "Parent Keypath");
  clip.add_required<std::string>("subpath", "Name of new keypath");
  clip.add_optional<std::string>("desc", "Description of new keypath", "");

  clip.add_required_state<std::map<std::string, std::string>>(
      kp_state_name, "Internal container for pending keypaths");
  clip.add_required_state<examplegraph::examplegraph>(
      graph_state_name, "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  std::string parstr = clip.get<keypath>("parent_keypath");
  auto parkp = keypath{parstr};
  auto subpath = clip.get<std::string>("subpath");
  auto desc = clip.get<std::string>("desc");

  std::string fullname = parstr + "." + subpath;

  // std::map<std::string, std::string> keypaths;
  auto the_graph = clip.get_state<examplegraph::examplegraph>(graph_state_name);

  if (parkp.headeq("edge")) {
    if (the_graph.has_edge_series(subpath)) {
      std::cerr << "!! ERROR: Keypath name already exists in edge table !!"
                << std::endl;
      exit(-1);
    }
  } else if (parkp.headeq("node")) {
    if (the_graph.has_node_series(subpath)) {
      std::cerr << "!! ERROR: Keypath name already exists in node table !!"
                << std::endl;
      exit(-1);
    }
  } else {
    std::cerr
        << "((!! ERROR: Parent must be either \"edge\" or \"node\" (received "
        << parstr << ") !!)";
    exit(-1);
  }

  if (clip.has_state(kp_state_name)) {
    auto keypaths =
        clip.get_state<std::map<std::string, std::string>>(kp_state_name);
    if (keypaths.contains(fullname)) {
      std::cerr << "Warning: Keypath name already exists; ignoring"
                << std::endl;
    } else {
      keypaths[fullname] = desc;
      clip.set_state(kp_state_name, keypaths);
      clip.update_keypaths(keypaths);
      clip.return_self();
    }
  }

  return 0;
}
