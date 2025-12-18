// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include <boost/json.hpp>
#include <iostream>
#include <map>
#include <string>

namespace boostjsn = boost::json;

static const std::string method_name = "__init__";
static const std::string state_name = "selector_state";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Initializes a TestSelector"};

  // no object-state requirements in constructor
  if (clip.parse(argc, argv)) {
    return 0;
  }

  std::map<std::string, std::string> map_selectors;
  clip.set_state(state_name, map_selectors);

  return 0;
}
