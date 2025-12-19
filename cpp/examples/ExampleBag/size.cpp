// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include <boost/json.hpp>
#include <cassert>
#include <iostream>

namespace boostjsn = boost::json;

static const std::string method_name = "size";
static const std::string state_name = "INTERNAL";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Returns the size of the bag"};

  clip.returns<size_t>("Size of bag.");

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto the_bag = clip.get_state<std::vector<int>>(state_name);

  clip.to_return<size_t>(the_bag.size());
  return 0;
}
