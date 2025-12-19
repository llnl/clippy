// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>

int main(int argc, char **argv) {
  clippy::clippy clip("call_with_optional_string", "Call With Optional String");
  clip.add_optional<std::string>("name", "Optional String", "World");
  clip.returns<std::string>("Returns a string");
  if (clip.parse(argc, argv)) {
    return 0;
  }

  auto name = clip.get<std::string>("name");

  clip.to_return(std::string("Howdy, ") + name);
  return 0;
}
