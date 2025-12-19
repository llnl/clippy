// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include "examplegraph.hpp"
#include <algorithm>
#include <boost/json.hpp>
#include <iostream>
#include <set>

namespace boostjsn = boost::json;

static const std::string method_name = "remove_edge";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {

  clippy::clippy clip{method_name, "Removes a string from a ExampleSet"};

  clip.add_required<int>("item", "Item to remove");
  clip.add_required_state<std::set<int>>(state_name, "Internal container");
  clip.returns_self();
  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto item = clip.get<int>("item");
  auto the_set = clip.get_state<std::set<int>>(state_name);
  the_set.erase(item);
  clip.set_state(state_name, the_set);
  clip.return_self();
  return 0;
}
