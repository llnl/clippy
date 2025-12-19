#pragma once
#include <deque>
#include <iostream>
#include <ranges>

#include "boost/json.hpp"

class selector {
  std::string sel_str;

  std::vector<size_t> dots;

  static std::vector<size_t> make_dots(const std::string &sel_str) {
    std::vector<size_t> dots;
    for (size_t i = 0; i < sel_str.size(); ++i) {
      if (sel_str[i] == '.') {
        dots.push_back(i);
      }
    }
    dots.push_back(sel_str.size());
    return dots;
  }

 public:
  friend std::ostream &operator<<(std::ostream &os, const selector &sel);
  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, const selector &sel);
  explicit selector(const std::string &sel)
      : sel_str(sel), dots(make_dots(sel_str)) {}
  explicit selector(const char *sel) : selector(std::string(sel)) {}
  selector(boost::json::object o) {
    auto v = o.at("rule").as_object()["var"];
    sel_str = v.as_string().c_str();
    dots = make_dots(sel_str);
  }
  bool operator<(const selector &other) const {
    return sel_str < other.sel_str;
  }
  operator std::string() { return sel_str; }
  operator std::string() const { return sel_str; }
  bool headeq(const std::string &comp) const {
    return std::string_view(sel_str).substr(0, dots[0]) == comp;
  }
  std::optional<selector> tail() const {
    if (dots.size() <= 1) {  // remember that end of string is a dot
      return std::nullopt;
    }
    return selector(sel_str.substr(dots[0] + 1));
  }
};

std::ostream &operator<<(std::ostream &os, const selector &sel) {
  os << sel.sel_str;
  return os;
}

selector tag_invoke(boost::json::value_to_tag<selector> /*unused*/,
                    const boost::json::value &v) {
  return v.as_object();
}

void tag_invoke(boost::json::value_from_tag /*unused*/, boost::json::value &v,
                const selector &sel) {
  std::cerr << "This should not be called." << std::endl;
  // std::map<std::string, std::string> o {};
  // o["expression_type"] = "jsonlogic";
  // o["rule"] = {{"var", sel.sel_str}};
  // v =  {"expression_type": "jsonlogic", "rule": {"var":
  // "node.degree"}}}sel.sel_str;
}