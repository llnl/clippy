// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>

int main(int argc, char **argv) {
  clippy::clippy clip("call_with_string", "Call With String");
  clip.add_required<std::string>("name", "Required String");
  clip.returns<std::string>("Returns a string");
  if (clip.parse(argc, argv)) { return 0; }

  auto name = clip.get<std::string>("name");

  clip.to_return(std::string("Howdy, ") + name);
  return 0;
}
