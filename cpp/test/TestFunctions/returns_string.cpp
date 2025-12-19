// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>

int main(int argc, char **argv) {
  clippy::clippy clip("returns_string", "Tests returning a string");
  clip.returns<std::string>("The string");
  if (clip.parse(argc, argv)) { return 0; }


  clip.to_return(std::string("asdf"));
  return 0;
}
