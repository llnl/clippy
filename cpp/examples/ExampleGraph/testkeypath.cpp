#include "../../include/clippy/keypath.hpp"
#include <iostream>

int main() {
  keypath s = keypath("foo.bar.baz");

  keypath zzz = "foo.zoo.boo";

  keypath s2{"x.y.z"};
  std::cout << "s = " << s << "\n";
  assert(s.headeq("foo"));
  assert(!s.headeq("bar"));

  auto val = boost::json::value_from(s);
  auto str = boost::json::serialize(val);

  auto t = boost::json::value_to<keypath>(val);
  assert(t.headeq("foo"));
  std::cout << str << "\n";
}
