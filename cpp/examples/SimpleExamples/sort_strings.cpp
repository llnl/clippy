// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#include <clippy/clippy.hpp>
#include <vector>
#include <algorithm>

int main(int argc, char **argv) {
  clippy::clippy clip("sort_strings", "Sorts an array of strings");
  clip.add_required<std::vector<std::string>>("strings",
                                         "Unordered array of strings");
  clip.add_optional<bool>("reverse", "Sort in reverse order", false);

  clip.returns<std::vector<std::string>>("Sorted array of strings");
  if (clip.parse(argc, argv)) { return 0; }

  auto strings = clip.get<std::vector<std::string>>("strings");
  bool reverse = clip.get<bool>("reverse");

  //  std::cout << "after reverse. It equals " << std::boolalpha << reverse <<
  //  std::endl;
  if (reverse) {
    std::sort(strings.begin(), strings.end(),
              std::greater<decltype(strings)::value_type>{});
  } else {
    std::sort(strings.begin(), strings.end());
  }

  clip.to_return(strings);
  return 0;
}
