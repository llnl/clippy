#pragma once
#include <boost/json/src.hpp>
#include <iostream>
#include <string>

namespace testgraph {
class selector {
  std::string name;

public:
  selector(boost::json::object &jo) {
    try {
      if (jo["expression_type"].as_string() != std::string("jsonlogic")) {
        std::cerr << " NOT A THINGY\n";
        exit(-1);
      }
      name = jo["rule"].as_object()["var"].as_string().c_str();
    } catch (...) {
      std::cerr << "!! ERROR !!\n";
      exit(-1);
    }
  }
  [[nodiscard]] std::string to_string() const { return name; }
};

} // namespace testgraph
