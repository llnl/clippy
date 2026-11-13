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

static const std::string method_name = "copy_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string kp_state_name = "keypaths";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Copies a subpath to a new subpath"};
  clip.add_required<keypath>("from_kp", "Source Keypath");
  clip.add_required<keypath>("to_kp", "Target Keypath");

  clip.add_required_state<std::map<keypath, std::string>>(
      kp_state_name, "Internal container for pending keypaths");
  clip.add_required_state<examplegraph::examplegraph>(
      graph_state_name, "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto from_keypath = clip.get<keypath>("from_kp");
  auto to_keypath = clip.get<keypath>("to_kp");

  auto the_graph = clip.get_state<examplegraph::examplegraph>(graph_state_name);

  bool edge_kp = false; // if false, then node keypath
  if (examplegraph::examplegraph::is_edge_keypath(from_keypath)) {
    edge_kp = true;
  } else if (!examplegraph::examplegraph::is_node_keypath(from_keypath)) {
    std::cerr << "!! ERROR: from_keypath must start with either \"edge\" or "
                 "\"node\" (received "
              << from_keypath << ") !!";
    exit(-1);
  }

  if (the_graph.has_series(to_keypath)) {
    std::cerr << "!! ERROR: Keypath name " << to_keypath
              << " already exists in graph !!" << std::endl;
    exit(-1);
  }

  if (clip.has_state(kp_state_name)) {
    auto keypaths =
        clip.get_state<std::map<keypath, std::string>>(kp_state_name);
    if (keypaths.contains(to_keypath)) {
      std::cerr << "Warning: Using unmanifested keypath." << std::endl;
      keypaths.erase(to_keypath);
    }
    auto from_keypath_tail = from_keypath.tail();
    auto to_keypath_tail = to_keypath.tail();
    if (!from_keypath_tail.has_value() || !to_keypath_tail.has_value()) {
      std::cerr
          << "!! ERROR: from_keypath and to_keypath must have content !!"
          << std::endl;
      exit(1);
    }
    from_keypath = from_keypath_tail.value();
    to_keypath = to_keypath_tail.value();
    if (edge_kp) {
      if (!the_graph.copy_edge_series(from_keypath, to_keypath)) {
        std::cerr << "!! ERROR: copy failed from " << from_keypath << " to "
                  << to_keypath << "!!" << std::endl;
        exit(1);
      };
    } else {
      if (!the_graph.copy_node_series(from_keypath, to_keypath)) {
        std::cerr << "!! ERROR: copy failed from " << from_keypath << " to "
                  << to_keypath << "!!" << std::endl;
        exit(1);
      };
    }
  }

  clip.set_state(graph_state_name, the_graph);
  clip.return_self();

  return 0;
}
