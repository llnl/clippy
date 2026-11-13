

// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <boost/json.hpp>
#include <clippy/clippy.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <queue>

#include "examplegraph.hpp"

static const std::string method_name = "connected_components";
static const std::string state_name = "INTERNAL";
static const std::string kp_state_name = "keypaths";

int main(int argc, char **argv) {
  clippy::clippy clip{
      method_name,
      "Populates a column containing the component id of each node in a graph"};
  clip.add_required<boost::json::object>(
      "keypath",
      "Existing keypath name into which the component id will be written");
  clip.add_required_state<examplegraph::examplegraph>(state_name,
                                                      "Internal container");
  clip.add_required_state<std::map<std::string, std::string>>(
      kp_state_name, "Internal container for keypaths");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto kp_json = clip.get<boost::json::object>("keypath");

  std::string kp;
  try {
    if (kp_json["expression_type"].as_string() != std::string("jsonlogic")) {
      std::cerr << " NOT A THINGY " << std::endl;
      exit(-1);
    }
    kp = kp_json["rule"].as_object()["var"].as_string().c_str();
  } catch (...) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }

  if (!kp.starts_with("node.")) {
    std::cerr << "Keypath must be a node subpath" << std::endl;
    return 1;
  }
  auto the_graph = clip.get_state<examplegraph::examplegraph>(state_name);

  auto keypaths =
      clip.get_state<std::map<std::string, std::string>>(kp_state_name);
  if (!keypaths.contains(kp)) {
    std::cerr << "Keypath not found" << std::endl;
    return 1;
  }
  auto subpath = kp.substr(5);
  if (the_graph.has_node_series(subpath)) {
    std::cerr << "Keypath already populated" << std::endl;
    return 1;
  }

  auto cc_o = the_graph.add_node_series<int64_t>(subpath, keypaths.at(kp));
  if (!cc_o) {
    std::cerr << "Unable to manifest node series" << std::endl;
    return 1;
  }

  auto cc = cc_o.value();
  std::map<examplegraph::node_t, int64_t> ccmap;

  int64_t i = 0;
  for (auto &node : the_graph.nodes()) {
    ccmap[node] = i++;
  }
  std::vector<std::vector<int64_t>> adj(the_graph.nv());
  the_graph.for_all_edges([&adj, &ccmap](auto edge, mvmap::locator /*unused*/) {
    long i = ccmap[edge.first];
    long j = ccmap[edge.second];
    adj[i].push_back(j);
    adj[j].push_back(i);
  });

  std::vector<bool> visited(the_graph.nv(), false);
  std::vector<int64_t> components(the_graph.nv());
  std::iota(components.begin(), components.end(), 0);

  for (size_t i = 0; i < the_graph.nv(); ++i) {
    if (!visited[i]) {
      std::queue<int64_t> q;
      q.push(int64_t(i));
      while (!q.empty()) {
        int64_t v = q.front();
        q.pop();
        visited[v] = true;
        for (int64_t u : adj[v]) {
          if (!visited[u]) {
            q.push(u);
            components[u] = components[i];
          }
        }
      }
    }
  }

  the_graph.for_all_nodes(
      [&components, &ccmap, &cc](auto node, mvmap::locator /*unused*/) {
        int64_t i = ccmap[node];
        cc[node] = components[i];
      });

  clip.set_state(state_name, the_graph);
  // clip.set_state(kp_state_name, keypaths);
  // clip.update_keypaths(keypaths);

  clip.return_self();
  return 0;
}
