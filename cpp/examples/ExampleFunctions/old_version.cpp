#include <iostream>

int main(int argc, char **arv) {
  std::cout
      << R"json({"method_name":"old_version","desc":"Tests returning a string - outdated version spec","version":"0.1.0","returns":{"desc":"The string"}})json";
  return 0;
}
