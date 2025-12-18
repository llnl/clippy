// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include <boost/json.hpp>

static const std::string class_name = "ClippyBag";
static const std::string method_name = "insert";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Inserts a string into a ClippyBag"};
  clip.add_required<int>("item", "Item to insert");
  clip.add_required_state<std::vector<int>>(state_name,
                                                    "Internal container");
  clip.returns_self();

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto item = clip.get<int>("item");
  auto the_bag = clip.get_state<std::vector<int>>(state_name);
  the_bag.push_back(item);
  clip.set_state(state_name, the_bag);
  clip.return_self();
  return 0;
}
