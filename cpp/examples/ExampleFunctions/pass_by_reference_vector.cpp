// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>

int main(int argc, char **argv) {
  clippy::clippy clip("pass_by_reference_vector", "Call with vector");
  clip.add_required<std::vector<size_t>>("vec", "Required vector");
  
  if (clip.parse(argc, argv)) { return 0; }

  std::vector<size_t> vec = {5,4,3,2,1};

  clip.overwrite_arg("vec", vec);
  return 0;
}
