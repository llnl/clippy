// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>

int main(int argc, char **argv) {
  clippy::clippy clip("pass_by_reference_dict", "Call with dict");
  clip.add_required<std::map<std::string, std::string>>("debug_info", "Required dict");
  
  if (clip.parse(argc, argv)) { return 0; }

  std::map<std::string, std::string> m = {{"dummy_key", "dummy_value"}};
  
  clip.overwrite_arg("debug_info", m);
  return 0;
}
