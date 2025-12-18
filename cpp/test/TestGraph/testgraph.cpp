#include "testgraph.hpp"

#include <iostream>

int main() {
  auto g = testgraph::testgraph{};

  g.add_node("a");
  g.add_node("b");
  g.add_edge("a", "b");
  g.add_edge("b", "c");

  assert(g.has_node("a"));
  assert(g.has_node("b"));
  assert(g.has_node("c"));
  assert(!g.has_node("d"));

  assert(g.has_edge({"a", "b"}));
  assert(g.has_edge({"b", "c"}));
  assert(!g.has_edge({"a", "c"}));

  assert(g.out_degree("a") == 1);
  assert(g.in_degree("a") == 0);
  assert(g.in_degree("b") == 1);
  assert(g.out_degree("b") == 1);

  auto colref =
      g.add_node_series<std::string>("color", "The color of the nodes");
  colref.value()["a"] = "blue";
  colref.value()["c"] = "red";

  auto weightref = g.add_edge_series<double>("weight", "edge weights");

  weightref.value()[std::pair("a", "b")] = 5.5;
  weightref.value()[std::pair("b", "c")] = 3.3;

  std::cout << "g.nv: " << g.nv() << ", g.ne: " << g.ne() << "\n";
  std::cout << boost::json::value_from(g) << "\n";

  auto val = boost::json::value_from(g);
  auto str = boost::json::serialize(val);
  std::cout << "here it is: " << str << "\n";

  auto g2 = boost::json::value_to<testgraph::testgraph>(val);
  auto e2 = weightref.value().extrema();
  std::cout << "extrema: " << std::get<0>(e2.first.value()) << ", "
            << std::get<0>(e2.second.value()) << "\n";
}
