
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
// #include <jsonlogic/src.hpp>
#include <variant>

#include "clippy/selector.hpp"
#include "testgraph.hpp"
#include "where.cpp"

static const std::string method_name = "dump2";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

static const std::string always_true = R"({"rule":{"==":[1,1]}})";
static const std::string never_true = R"({"rule":{"==":[2,1]}})";

static const boost::json::object always_true_obj =
    boost::json::parse(always_true).as_object();

int main(int argc, char **argv) {
  std::cerr << "starting" << std::endl;
  clippy::clippy clip{method_name,
                      "returns a map of key: value "
                      "corresponding to the selector."};
  clip.add_required<selector>("selector",
                              "Existing selector name to obtain values");

  clip.add_optional<boost::json::object>("where", "where filter",
                                         always_true_obj);

  clip.add_required_state<testgraph::testgraph>(state_name,
                                                "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  std::cerr << "past parse" << std::endl;

  auto sel_str = clip.get<selector>("selector");
  selector sel{sel_str};

  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);

  bool is_edge_sel = testgraph::testgraph::is_edge_selector(sel);
  bool is_node_sel = testgraph::testgraph::is_node_selector(sel);

  if (!is_edge_sel && !is_node_sel) {
    std::cerr << "Selector must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  std::cerr << "before tailsel_opt" << std::endl;
  auto tailsel_opt = sel.tail();
  if (!tailsel_opt) {
    std::cerr << "no tail" << std::endl;
    return 1;
  }

  auto expression = clip.get<boost::json::object>("where");
  // auto expression = boost::json::parse(always_true).as_object();
  std::cerr << "expression: " << expression << std::endl;

  std::string tail_sel = tailsel_opt.value();
  if (is_node_sel) {
    clip.returns<std::vector<testgraph::node_t>>(
        "vector of node keys that match the selector");
    auto filtered_data = where_nodes(the_graph, expression);
    clip.to_return(filtered_data);
    return 0;
  }

  clip.returns<std::map<std::string, bool>>(
      "map of key: value corresponding to the selector");

  clip.to_return(std::map<std::string, bool>{});
  clip.set_state(state_name, the_graph);
  return 0;
}
