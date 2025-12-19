// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <list>

#include "testgraph.hpp"

static const std::string method_name = "add_node";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name,
                      "Adds a new column to a graph based on a lambda"};
  clip.add_required<boost::json::object>("name", "New column name");
  clip.add_required<boost::json::object>("expression", "Lambda Expression");
  clip.add_required_state<testgraph::testgraph>(state_name,
                                                "Internal container");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto name = clip.get<boost::json::object>("name");
  auto expression = clip.get<boost::json::object>("expression");
  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);

  //
  // Expression here
  auto apply_jl = [&expression](const testgraph::edge_t &value,
                                mvmap::locator loc) {
    boost::json::object data;
    data["src"] = value.first;
    data["dst"] = value.second;
    data["loc"] = boost::json::value_from(loc);
    jsonlogic::any_expr res = jsonlogic::apply(expression["rule"], data);
    return jsonlogic::unpack_value<bool>(res);
  };

  the_graph.for_all_edges(apply_jl);

  clip.set_state(state_name, the_graph);
  clip.return_self();
  return 0;
}
