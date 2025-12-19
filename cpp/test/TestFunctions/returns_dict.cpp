// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <unordered_map>

int main(int argc, char **argv) {
  clippy::clippy clip("returns_dict", "Tests returning a dict");
  clip.returns<std::unordered_map<std::string, int>>("The Dict");
  if (clip.parse(argc, argv)) {
    return 0;
  }

  std::unordered_map<std::string, int> m1{{"a", 1}, {"b", 2}, {"c", 3}};
  // std::unordered_map<std::string, std::unordered_map<std::string, int>>
  // big_m;
  clip.to_return(m1);
  return 0;
}
