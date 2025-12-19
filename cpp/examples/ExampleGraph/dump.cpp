
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <variant>

#include "clippy/selector.hpp"
#include "examplegraph.hpp"

static const std::string method_name = "dump";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

static const std::string always_true = R"({"rule":{"==":[1,1]}})";
static const std::string never_true = R"({"rule":{"==":[2,1]}})";

static const boost::json::object always_true_obj =
    boost::json::parse(always_true).as_object();

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "returns a map of key: value "
                                   "corresponding to the selector."};
  clip.add_required<selector>("selector",
                              "Existing selector name to obtain values");

  clip.add_optional<boost::json::object>("where", "where filter",
                                         always_true_obj);

  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto sel_str = clip.get<selector>("selector");
  selector sel{sel_str};

  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  bool is_edge_sel = examplegraph::examplegraph::is_edge_selector(sel);
  bool is_node_sel = examplegraph::examplegraph::is_node_selector(sel);

  if (!is_edge_sel && !is_node_sel) {
    std::cerr << "Selector must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  auto tailsel_opt = sel.tail();
  if (!tailsel_opt) {
    std::cerr << "no tail" << std::endl;
    return 1;
  }

  auto expression = clip.get<boost::json::object>("where");
  // auto expression = boost::json::parse(always_true).as_object();
  std::cerr << "expression: " << expression << std::endl;

  // we need to make a copy here because translateNode modifies the object
  boost::json::object exp2(expression);
  auto [_a /*unused*/, vars, _b /*unused*/] =
      jsonlogic::translateNode(exp2["rule"]);
  std::cerr << "post-translate expression: " << expression << std::endl;
  std::cerr << "post-translate expression['rule']: " << expression["rule"]
            << std::endl;
  auto apply_jl = [&expression, &vars](int value) {
    boost::json::object data;
    boost::json::value val = value;
    std::cerr << "    apply_jl expression: " << expression << std::endl;
    for (auto var : vars) {
      data[var] = val;
      std::cerr << "    apply_jl: var: " << var << " val: " << val << std::endl;
    }

    jsonlogic::any_expr res = jsonlogic::apply(expression["rule"], data);
    std::cerr << "    apply_jl: res: " << res << std::endl;
    return jsonlogic::unpack_value<bool>(res);
  };

  std::string tail_sel = tailsel_opt.value();
  if (is_node_sel) {
    if (the_graph.has_node_series<int64_t>(tail_sel)) {
      auto pxy = the_graph.get_node_series<int64_t>(tail_sel).value();
      std::map<std::string, int64_t> filtered_data;
      pxy.for_all(
          [&filtered_data, &apply_jl](const auto &key, auto, const auto &val) {
            std::cerr << "key: " << key << " val: " << val << std::endl;
            if (apply_jl(val)) { // apply the where clause
              filtered_data[key] = val;
              std::cerr << "applied!" << std::endl;
            }
          });

      clip.returns<std::map<std::string, int64_t>>(
          "map of key: value corresponding to the selector");
      clip.to_return(filtered_data);
      return 0;
    } else if (the_graph.has_node_series<bool>(tail_sel)) {
      // clip.returns<std::map<std::string, int64_t>>(
      //     "map of key: value corresponding to the selector");
    } else if (the_graph.has_node_series<double>(tail_sel)) {
      // clip.returns<std::map<std::string, double>>(
      //     "map of key: value corresponding to the selector");
    } else if (the_graph.has_node_series<std::string>(tail_sel)) {
      // clip.returns<std::map<std::string, std::string>>(
      //     "map of key: value corresponding to the selector");
    } else {
      std::cerr << "Node series is an invalid type" << std::endl;
      return 1;
    }
  }

  clip.returns<std::map<std::string, bool>>(
      "map of key: value corresponding to the selector");

  clip.to_return(std::map<std::string, bool>{});
  clip.set_state(state_name, the_graph);
  return 0;
}
