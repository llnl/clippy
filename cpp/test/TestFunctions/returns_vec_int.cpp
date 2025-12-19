// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>

int main(int argc, char **argv) {
  clippy::clippy clip("returns_vec_int", "Tests returning a vector of int");
  clip.returns<std::vector<size_t>>("The vec");
  if (clip.parse(argc, argv)) { return 0; }

  std::vector<size_t> to_return = {0,1,2,3,4,5};
  clip.to_return(to_return);
  return 0;
}
