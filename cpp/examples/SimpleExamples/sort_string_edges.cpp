// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <vector>
#include <algorithm>

using edge_array_t = std::vector<std::pair<std::string, std::string>>;

int main(int argc, char **argv) {
  clippy::clippy clip("sort_string_edges", "Sorts a string edgelist");
  clip.add_required<edge_array_t>("edges", "Unordered edgelist");
  clip.add_optional<bool>("reverse", "Sort in reverse order", false);

  clip.returns<edge_array_t>("Sorted edgelist");
  if (clip.parse(argc, argv)) { return 0; }

  auto edges = clip.get<edge_array_t>("edges");
  bool reverse = clip.get<bool>("reverse");

  if (reverse) {
    std::sort(edges.begin(), edges.end(),
              std::greater<decltype(edges)::value_type>{});
  } else {
    std::sort(edges.begin(), edges.end());
  }

  clip.to_return(edges);
  return 0;
}
