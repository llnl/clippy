
// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>

#include "clippy/selector.hpp"
#include "examplegraph.hpp"

static const std::string method_name = "extrema";
static const std::string state_name = "INTERNAL";
static const std::string sel_state_name = "selectors";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name,
                      "returns the extrema of a series based on selector"};
  clip.add_required<selector>("selector",
                              "Existing selector name to calculate extrema");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }
  auto sel_str = clip.get<selector>("selector");
  selector sel{sel_str};

  bool is_edge_sel = examplegraph::examplegraph::is_edge_selector(sel);
  bool is_node_sel = examplegraph::examplegraph::is_node_selector(sel);

  if (!is_edge_sel && !is_node_sel) {
    std::cerr << "Selector must start with either \"edge\" or \"node\""
              << std::endl;
    return 1;
  }

  auto tail_opt = sel.tail();
  if (!tail_opt) {
    std::cerr << "Selector must have a tail" << std::endl;
    return 1;
  }
  auto tail_sel = tail_opt.value();

  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  if (is_edge_sel) {
    clip.returns<
        std::map<std::string, std::pair<examplegraph::edge_t, double>>>(
        "min and max keys and values of the series");
    if (the_graph.has_edge_series<double>(tail_sel)) {
      auto series = the_graph.get_edge_series<double>(tail_sel);
      if (!series) {
        std::cerr << "Edge series not found" << std::endl;
        return 1;
      }
      auto series_val = series.value();
      auto [min_tup, max_tup] = series_val.extrema();

      std::map<std::string, std::pair<examplegraph::edge_t, double>> extrema;
      if (min_tup) {
        extrema["min"] = std::make_pair(std::get<1>(min_tup.value()),
                                        std::get<0>(min_tup.value()));
      }

      if (max_tup) {
        extrema["max"] = std::make_pair(std::get<1>(max_tup.value()),
                                        std::get<0>(max_tup.value()));
      }
      clip.to_return(extrema);
    } else if (the_graph.has_edge_series<int64_t>(tail_sel)) {
      auto series = the_graph.get_edge_series<int64_t>(tail_sel);
      if (!series) {
        std::cerr << "Edge series not found" << std::endl;
        return 1;
      }
      auto series_val = series.value();
      auto [min_tup, max_tup] = series_val.extrema();

      std::map<std::string, std::pair<examplegraph::edge_t, double>> extrema;
      if (min_tup) {
        extrema["min"] = std::make_pair(std::get<1>(min_tup.value()),
                                        std::get<0>(min_tup.value()));
      }

      if (max_tup) {
        extrema["max"] = std::make_pair(std::get<1>(max_tup.value()),
                                        std::get<0>(max_tup.value()));
      }
      clip.to_return(extrema);
    } else {
      std::cerr << "Edge series is an invalid type" << std::endl;
      return 1;
    }
  } else if (is_node_sel) {
    if (the_graph.has_edge_series<double>(tail_sel)) {
      clip.returns<
          std::map<std::string, std::pair<examplegraph::node_t, double>>>(
          "min and max keys and values of the series");

      auto series = the_graph.get_node_series<int64_t>(tail_sel);
      if (!series) {
        std::cerr << "Edge series not found" << std::endl;
        return 1;
      }

      auto series_val = series.value();
      auto [min_tup, max_tup] = series_val.extrema();

      std::map<std::string, std::pair<examplegraph::node_t, double>> extrema;
      if (min_tup) {
        extrema["min"] = std::make_pair(std::get<1>(min_tup.value()),
                                        std::get<0>(min_tup.value()));
      }
      if (max_tup) {
        extrema["max"] = std::make_pair(std::get<1>(max_tup.value()),
                                        std::get<0>(max_tup.value()));
      }

      clip.to_return(extrema);
    } else if (the_graph.has_node_series<int64_t>(tail_sel)) {
      clip.returns<
          std::map<std::string, std::pair<examplegraph::node_t, int64_t>>>(
          "min and max keys and values of the series");

      auto series = the_graph.get_node_series<int64_t>(tail_sel);
      if (!series) {
        return 1;
      }
      auto series_val = series.value();
      auto [min_tup, max_tup] = series_val.extrema();

      std::map<std::string, std::pair<examplegraph::node_t, int64_t>> extrema;
      if (min_tup) {
        extrema["min"] = std::make_pair(std::get<1>(min_tup.value()),
                                        std::get<0>(min_tup.value()));
      }
      if (max_tup) {
        extrema["max"] = std::make_pair(std::get<1>(max_tup.value()),
                                        std::get<0>(max_tup.value()));
      }

      clip.to_return(extrema);
    } else {
      std::cerr << "Node series is an invalid type" << std::endl;
      return 1;
    }
  }

  clip.set_state(state_name, the_graph);
  return 0;
}
