// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <cassert>
#include <iostream>
#include <variant>

#include "clippy/clippy.hpp"
#include "clippy/selector.hpp"
#include "testgraph.hpp"

namespace boostjsn = boost::json;

static const std::string method_name = "copy_series";
static const std::string graph_state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Copies a subselector to a new subselector"};
  clip.add_required<selector>("from_sel", "Source Selector");
  clip.add_required<selector>("to_sel", "Target Selector");

  clip.add_required_state<std::map<selector, std::string>>(
      sel_state_name, "Internal container for pending selectors");
  clip.add_required_state<testgraph::testgraph>(graph_state_name,
                                                "Internal state for the graph");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto from_selector = clip.get<selector>("from_sel");
  auto to_selector = clip.get<selector>("to_sel");

  auto the_graph = clip.get_state<testgraph::testgraph>(graph_state_name);

  bool edge_sel = false;  // if false, then node selector
  if (testgraph::testgraph::is_edge_selector(from_selector)) {
    edge_sel = true;
  } else if (!testgraph::testgraph::is_node_selector(from_selector)) {
    std::cerr << "!! ERROR: from_selector must start with either \"edge\" or "
                 "\"node\" (received "
              << from_selector << ") !!";
    exit(-1);
  }

  if (the_graph.has_series(to_selector)) {
    std::cerr << "!! ERROR: Selector name " << to_selector
              << " already exists in graph !!" << std::endl;
    exit(-1);
  }

  if (clip.has_state(sel_state_name)) {
    auto selectors =
        clip.get_state<std::map<selector, std::string>>(sel_state_name);
    if (selectors.contains(to_selector)) {
      std::cerr << "Warning: Using unmanifested selector." << std::endl;
      selectors.erase(to_selector);
    }
    auto from_selector_tail = from_selector.tail();
    auto to_selector_tail = to_selector.tail();
    if (!from_selector_tail.has_value() || !to_selector_tail.has_value()) {
      std::cerr
          << "!! ERROR: from_selector and to_selector must have content !!"
          << std::endl;
      exit(1);
    }
    from_selector = from_selector_tail.value();
    to_selector = to_selector_tail.value();
    if (edge_sel) {
      if (!the_graph.copy_edge_series(from_selector, to_selector)) {
        std::cerr << "!! ERROR: copy failed from " << from_selector << " to "
                  << to_selector << "!!" << std::endl;
        exit(1);
      };
    } else {
      if (!the_graph.copy_node_series(from_selector, to_selector)) {
        std::cerr << "!! ERROR: copy failed from " << from_selector << " to "
                  << to_selector << "!!" << std::endl;
        exit(1);
      };
    }
  }

  clip.set_state(graph_state_name, the_graph);
  clip.return_self();

  return 0;
}
