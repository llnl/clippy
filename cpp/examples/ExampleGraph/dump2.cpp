
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

#include "clippy/keypath.hpp"
#include "examplegraph.hpp"
#include "where.cpp"

static const std::string method_name = "dump2";
static const std::string state_name = "INTERNAL";
static const std::string kp_state_name = "keypaths";

static const std::string always_true = R"({"rule":{"==":[1,1]}})";
static const std::string never_true = R"({"rule":{"==":[2,1]}})";

static const boost::json::object always_true_obj =
    boost::json::parse(always_true).as_object();

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "returns a map of key: value "
                                   "corresponding to the keypath."};
  clip.add_required<keypath>("keypath",
                              "Existing keypath name to obtain values");

  clip.add_optional<boost::json::object>("where", "where filter",
                                         always_true_obj);

  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  std::cerr << "past parse" << std::endl;

  auto kp_str = clip.get<keypath>("keypath");
  keypath kp{kp_str};

  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  bool is_edge_kp = examplegraph::examplegraph::is_edge_keypath(kp);
  bool is_node_kp = examplegraph::examplegraph::is_node_keypath(kp);

  if (!is_edge_kp && !is_node_kp) {
    std::cerr << "Keypath must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  std::cerr << "before tailkp_opt" << std::endl;
  auto tailkp_opt = kp.tail();
  if (!tailkp_opt) {
    std::cerr << "no tail" << std::endl;
    return 1;
  }

  auto expression = clip.get<boost::json::object>("where");
  // auto expression = boost::json::parse(always_true).as_object();
  std::cerr << "expression: " << expression << std::endl;

  std::string tail_kp = tailkp_opt.value();
  if (is_node_kp) {
    clip.returns<std::vector<examplegraph::node_t>>(
        "vector of node keys that match the keypath");
    auto filtered_data = where_nodes(the_graph, expression);
    clip.to_return(filtered_data);
    return 0;
  }

  clip.returns<std::map<std::string, bool>>(
      "map of key: value corresponding to the keypath");

  clip.to_return(std::map<std::string, bool>{});
  clip.set_state(state_name, the_graph);
  return 0;
}
