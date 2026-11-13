// Copyright 2021 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include "clippy/clippy.hpp"
#include <boost/json.hpp>
#include <cassert>
#include <iostream>

namespace boostjsn = boost::json;

static const std::string method_name = "add";

int main(int argc, char **argv) {
  clippy::clippy clip{method_name, "Adds a subpath"};
  clip.add_required<boostjsn::object>("keypath", "Parent Keypath");
  clip.add_required<std::string>("subname", "Description of new keypath");
  clip.add_optional<std::string>("desc", "Description", "EMPTY DESCRIPTION");
  clip.add_required_state<std::map<std::string, std::string>>(
      "keypath_state", "Internal container");

  if (clip.parse(argc, argv)) {
    return 0;
  }

  std::map<std::string, std::string> sstate;
  if (clip.has_state("keypath_state")) {
    sstate =
        clip.get_state<std::map<std::string, std::string>>("keypath_state");
  }

  auto jo = clip.get<boostjsn::object>("keypath");
  std::string subname = clip.get<std::string>("subname");
  std::string desc = clip.get<std::string>("desc");

  std::string parentname;
  try {
    if (jo["expression_type"].as_string() != std::string("jsonlogic")) {
      std::cerr << " NOT A THINGY " << std::endl;
      exit(-1);
    }
    parentname = jo["rule"].as_object()["var"].as_string().c_str();
  } catch (...) {
    std::cerr << "!! ERROR !!" << std::endl;
    exit(-1);
  }

  sstate[parentname + "." + subname] = desc;

  clip.set_state("keypath_state", sstate);
  clip.update_keypaths(sstate);
  clip.return_self();

  return 0;
}
