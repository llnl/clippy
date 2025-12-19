#include <iostream>

int main(int argc, char **arv) {
  std::cout
      << R"json({"method_name":"missing_version","desc":"Tests returning a string - missing a version spec","returns":{"desc":"The string"}})json";
  return 0;
}
