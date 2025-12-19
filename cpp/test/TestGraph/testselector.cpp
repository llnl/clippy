#include "../../include/clippy/selector.hpp"
#include <iostream>

int main() {
  selector s = selector("foo.bar.baz");

  selector zzz = "foo.zoo.boo";

  selector s2{"x.y.z"};
  std::cout << "s = " << s << "\n";
  assert(s.headeq("foo"));
  assert(!s.headeq("bar"));

  auto val = boost::json::value_from(s);
  auto str = boost::json::serialize(val);

  auto t = boost::json::value_to<selector>(val);
  assert(t.headeq("foo"));
  std::cout << str << "\n";
}
