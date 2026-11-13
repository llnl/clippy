#pragma once

#include <iostream>

#include "boost/json.hpp"

class keypath {
  std::string kp_str;

  std::vector<size_t> dots;

  static std::vector<size_t> make_dots(const std::string &kp_str) {
    std::vector<size_t> dots;
    for (size_t i = 0; i < kp_str.size(); ++i) {
      if (kp_str[i] == '.') {
        dots.push_back(i);
      }
    }
    dots.push_back(kp_str.size());
    return dots;
  }

 public:
  friend std::ostream &operator<<(std::ostream &os, const keypath &kp);
  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, const keypath &kp);
  explicit keypath(const std::string &kp)
      : kp_str(kp), dots(make_dots(kp_str)) {}
  explicit keypath(const char *kp) : keypath(std::string(kp)) {}
  keypath(boost::json::object o) {
    auto v = o.at("rule").as_object()["var"];
    kp_str = v.as_string().c_str();
    dots = make_dots(kp_str);
  }
  bool operator<(const keypath &other) const {
    return kp_str < other.kp_str;
  }
  operator std::string() { return kp_str; }
  operator std::string() const { return kp_str; }
  bool headeq(const std::string &comp) const {
    return std::string_view(kp_str).substr(0, dots[0]) == comp;
  }
  std::optional<keypath> tail() const {
    if (dots.size() <= 1) {  // remember that end of string is a dot
      return std::nullopt;
    }
    return keypath(kp_str.substr(dots[0] + 1));
  }
};

inline std::ostream &operator<<(std::ostream &os, const keypath &kp) {
  os << kp.kp_str;
  return os;
}

inline keypath tag_invoke(boost::json::value_to_tag<keypath> /*unused*/,
                           const boost::json::value &v) {
  return v.as_object();
}

// void tag_invoke([[maybe_unused]] boost::json::value_from_tag /*unused*/,
//                 [[maybe_unused]] boost::json::value &v,
//                 [[maybe_unused]] const keypath &kp /*unused*/) {
//   std::cerr << "This should not be called." << std::endl;
// std::map<std::string, std::string> o {};
// o["expression_type"] = "jsonlogic";
// o["rule"] = {{"var", kp.kp_str}};
// v =  {"expression_type": "jsonlogic", "rule": {"var":
// "node.degree"}}}kp.kp_str;
//}