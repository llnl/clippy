#include "mvmap.hpp"
#include <boost/json/src.hpp>
// #include <boost/json/serialize_options.hpp>
#include <cassert>
#include <iostream>
#include <string>

using mymap_t = mvmap::mvmap<std::string, bool, double, int64_t, std::string>;
int main() {

  mymap_t m{};

  m.add_series<double>("weight");
  std::cout << "added series weight\n";
  m.add_series<std::string>("name");
  std::cout << "added series name\n";

  auto hmap = m.get_or_create_series<double>("age");
  std::cout << "created hmap\n";

  hmap["seth"] = 5;
  std::cout << "added seth\n";
  hmap["roger"] = 8;
  std::cout << "added roger\n";

  assert(hmap["seth"] == 5);
  assert(hmap["roger"] == 8);

  auto v = boost::json::value_from(m);
  std::string j = boost::json::serialize(v);
  std::cout << "j = " << j << '\n';
  auto jv = boost::json::parse(j);
  std::cout << boost::json::serialize(jv) << "\n";
  auto n = boost::json::value_to<mymap_t>(jv);

  std::cout << "created n\n";
  auto hmap2 = n.get_or_create_series<double>("age");
  std::cout << "created hmap2\n";
  assert(hmap2["seth"] == 5);
  assert(hmap2["roger"] == 8);

  size_t age_sum = 0;
  hmap2.for_all([&age_sum](const auto &k, auto, auto &v) { age_sum += v; });

  std::cout << "sum of ages = " << age_sum << "\n";
  assert(age_sum == 13);

  hmap2.remove_if([](const auto &k, auto, auto &v) { return v > 6; });

  assert(hmap2.at("roger") == std::nullopt);
  assert(hmap2.at("seth") == 5);

  age_sum = 0;
  hmap2.for_all([&age_sum](const auto &k, auto, auto &v) { age_sum += v; });
  std::cout << "sum of ages = " << age_sum << "\n";
  assert(age_sum == 5);
  hmap2["roger"] = 8;

  age_sum = 0;
  hmap2.for_all([&age_sum](const auto &k, auto, auto &v) { age_sum += v; });

  std::cout << "sum of ages = " << age_sum << "\n";
  assert(age_sum == 13);
  n.remove_if([&hmap2](const auto &k, auto) { return hmap2[k] == 5; });

  age_sum = 0;
  hmap2.for_all([&age_sum](const auto &k, auto, auto &v) { age_sum += v; });

  std::cout << "sum of ages = " << age_sum << "\n";
  assert(age_sum == 8);
}
