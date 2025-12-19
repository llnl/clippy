

// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <queue>

#include "clippy/selector.hpp"
#include "testgraph.hpp"
#include "where.cpp"

static const std::string method_name = "assign";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

static const std::string always_true = R"({"rule":{"==":[1,1]}})";
static const std::string never_true = R"({"rule":{"==":[2,1]}})";

static const boost::json::object always_true_obj =
    boost::json::parse(always_true).as_object();

using variants =
    std::variant<bool, double, int64_t, std::string, boost::json::object>;

std::optional<boost::json::value> obj_to_val(boost::json::object expr,
                                             std::string extract = "var") {
  if (expr["expression_type"].as_string() != std::string("jsonlogic")) {
    std::cerr << " NOT A THINGY " << std::endl;
    return std::nullopt;
  }

  boost::json::value v;
  v = expr["rule"].as_object()[extract];
  return v;
}
int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Populates a column with a value"};
  clip.add_required<boost::json::object>(
      "selector",
      "Existing selector name into which the value will be written");

  clip.add_required<variants>("value", "the value to assign");

  clip.add_optional<boost::json::object>("where", "where filter",
                                         always_true_obj);

  clip.add_optional<std::string>("desc", "Description of the series", "");

  clip.add_required_state<testgraph::testgraph>(state_name,
                                                "Internal container");
  clip.add_required_state<std::map<std::string, std::string>>(
      sel_state_name, "Internal container for selectors");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto sel_json = clip.get<boost::json::object>("selector");
  auto sel_str_opt = obj_to_val(sel_json);
  if (!sel_str_opt.has_value()) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }
  std::string sel_str = sel_str_opt.value().as_string().c_str();

  auto sel = selector(sel_str);
  bool is_node_sel = sel.headeq("node");
  if (!is_node_sel && !sel.headeq("edge")) {
    std::cerr << "Selector must start with \"node\" or \"edge\"" << std::endl;
    return 1;
  }

  auto sel_tail_opt = sel.tail();
  if (!sel_tail_opt.has_value()) {
    std::cerr << "Selector must have a tail" << std::endl;
    return 1;
  }

  selector subsel = sel_tail_opt.value();
  auto the_graph = clip.get_state<testgraph::testgraph>(state_name);

  auto selectors =
      clip.get_state<std::map<std::string, std::string>>(sel_state_name);
  if (!selectors.contains(sel)) {
    std::cerr << "Selector not found" << std::endl;
    return 1;
  }

  auto desc = clip.get<std::string>("desc");

  auto val = clip.get<boost::json::value>("value");

  auto where_exp = clip.get<boost::json::object>("where");

  if (where_exp["expression_type"].as_string() != std::string("jsonlogic")) {
    std::cerr << "!! ERROR in where statement !!" << std::endl;
    exit(-1);
  }

  boost::json::object submission_data;

  std::cerr << "val = " << val << ", val.kind() = " << val.kind() << std::endl;
  if (is_node_sel) {
    if (the_graph.has_node_series(subsel)) {
      std::cerr << "Selector already populated" << std::endl;
      return 1;
    }

    auto nodemap = the_graph.nodemap();
    auto where = parse_where_expression(nodemap, where_exp, submission_data);
    if (std::holds_alternative<bool>(val)) {
      auto col_opt = the_graph.add_node_series<bool>(subsel, desc);
      if (!col_opt.has_value()) {
        std::cerr << "Unable to manifest node series" << std::endl;
        return 1;
      }
      auto col = col_opt.value();
      auto v = std::get<bool>(val);
      the_graph.for_all_nodes(
          [&col, &v, &where](auto /* unused */, mvmap::locator l) {
            if (where(l)) {
              col[l] = v;
            }
          });
    } else if (std::holds_alternative<double>(val)) {
      auto col_opt = the_graph.add_node_series<double>(subsel, desc);
      if (!col_opt.has_value()) {
        std::cerr << "Unable to manifest node series" << std::endl;
        return 1;
      }
      auto col = col_opt.value();
      auto v = std::get<double>(val);
      the_graph.for_all_nodes(
          [&col, &v, &where](auto /* unused */, mvmap::locator l) {
            if (where(l)) {
              col[l] = v;
            }
          });
    } else if (std::holds_alternative<int64_t>(val)) {
      auto col_opt = the_graph.add_node_series<int64_t>(subsel, desc);
      if (!col_opt.has_value()) {
        std::cerr << "Unable to manifest node series" << std::endl;
        return 1;
      }
      auto col = col_opt.value();
      auto v = std::get<int64_t>(val);
      the_graph.for_all_nodes(
          [&col, &v, &where](auto /* unused */, mvmap::locator l) {
            if (where(l)) {
              col[l] = v;
            }
          });
    } else if (std::holds_alternative<std::string>(val)) {
      auto col_opt = the_graph.add_node_series<std::string>(subsel, desc);
      if (!col_opt.has_value()) {
        std::cerr << "Unable to manifest node series" << std::endl;
        return 1;
      }
      auto col = col_opt.value();
      auto v = std::get<std::string>(val).c_str();
      the_graph.for_all_nodes(
          [&col, &v, &where](auto /* unused */, mvmap::locator l) {
            if (where(l)) {
              col[l] = v;
            }
          });
    } else if (std::holds_alternative<boost::json::object>(val)) {
      auto col_opt =
          the_graph.add_node_series<boost::json::object>(subsel, desc);
      if (!col_opt.has_value()) {
        std::cerr << "Unable to manifest node series" << std::endl;
        return 1;
      }
      case boost::json::kind::bool_: {
        auto col_opt = the_graph.add_node_series<bool>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest node series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_bool();
        the_graph.for_all_nodes(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });

        break;
      }
      case boost::json::kind::double_: {
        auto col_opt = the_graph.add_node_series<double>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest node series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_double();
        the_graph.for_all_nodes(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });

        break;
      }

      case boost::json::kind::int64: {
        auto col_opt = the_graph.add_node_series<int64_t>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest node series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_int64();
        the_graph.for_all_nodes(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });
        break;
      }

      case boost::json::kind::string: {
        auto col_opt = the_graph.add_node_series<std::string>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest node series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_string().c_str();
        the_graph.for_all_nodes(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });
        break;
      }
      default:
        std::cerr << "Unsupported type" << std::endl;
        return 1;
    }
  } else {  // edge map
    if (the_graph.has_edge_series(subsel)) {
      std::cerr << "Selector already populated" << std::endl;
      return 1;
    }

    auto edgemap = the_graph.edgemap();
    auto where = parse_where_expression(edgemap, where_exp, submission_data);
    switch (val.kind()) {
      case boost::json::kind::bool_: {
        auto col_opt = the_graph.add_edge_series<bool>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest edge series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_bool();
        the_graph.for_all_edges(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });
        break;
      }
      case boost::json::kind::double_: {
        auto col_opt = the_graph.add_edge_series<double>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest edge series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_double();
        the_graph.for_all_edges(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });
        break;
      }

      case boost::json::kind::int64: {
        auto col_opt = the_graph.add_edge_series<int64_t>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest edge series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_int64();
        the_graph.for_all_edges(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });

        break;
      }

      case boost::json::kind::string: {
        auto col_opt = the_graph.add_edge_series<std::string>(subsel, desc);
        if (!col_opt.has_value()) {
          std::cerr << "Unable to manifest edge series" << std::endl;
          return 1;
        }
        auto col = col_opt.value();
        auto v = val.as_string().c_str();
        the_graph.for_all_edges(
            [&col, &v, &where](auto /* unused */, mvmap::locator l) {
              if (where(l)) {
                col[l] = v;
              }
            });
        break;
      }
      default:
        std::cerr << "Unsupported type" << std::endl;
        return 1;
    }
  }

  clip.set_state(state_name, the_graph);
  clip.set_state(sel_state_name, selectors);
  clip.update_selectors(selectors);

  clip.return_self();
  return 0;
}
