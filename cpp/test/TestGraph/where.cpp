#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <iostream>
#include <jsonlogic/src.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "clippy/selector.hpp"
#include "jsonlogic/logic.hpp"
#include "testgraph.hpp"

template <typename M>
auto parse_where_expression(M& mvmap_, boost::json::object& expression,
                            boost::json::object& submission_data) {
  // std::cerr << "  parse_where_expression: expression: " << expression
  //           << std::endl;
  boost::json::object exp2(expression);

  std::shared_ptr<jsonlogic::logic_rule> jlrule =
      std::make_shared<jsonlogic::logic_rule>(
          jsonlogic::create_logic(exp2["rule"]));
  // std::cerr << "past create_logic\n";
  auto vars = jlrule->variable_names();
  // boost::json::object submission_data{};
  // jsonlogic::any_expr expression_rule_;

  // we use expression_rule_ (and expression_rule; see below) in order to avoid
  // having to recompute this every time we call the lambda.
  // std::tie(expression_rule_, vars, std::ignore) =
  //     jsonlogic::create_logic(exp2["rule"]);

  // this works around a deficiency in C++ compilers where
  // unique pointers moved into a lambda cannot be moved into
  // an std::function.
  // jsonlogic::expr* rawexpr = expression_rule_.release();
  // std::shared_ptr<jsonlogic::expr> expression_rule{rawexpr};
  // std::shared_ptr<jsonlogic::logic_rule> jlshared{&jlrule};

  // std::cerr << "parse_where: # of vars: " << vars.size() << std::endl;
  // for (const auto& var : vars) {
  //   std::cerr << "  apply_jl var dump: var: " << var << std::endl;
  // }

  auto apply_jl = [&expression, jlrule, &mvmap_,
                   &submission_data](mvmap::locator loc) mutable {
    auto vars = jlrule->variable_names();
    // std::cerr << "    apply_jl:  # of vars: " << vars.size() << std::endl;
    for (const auto& var : vars) {
      // std::cerr << "    apply_jl: var: " << var << std::endl;
      auto var_sel = selector(std::string(var));
      // std::cerr << "  apply_jl: var_sel = " << var_sel << std::endl;
      // if (!var_sel.headeq("node")) {
      //   std::cerr << "selector is not a node selector; skipping." <<
      //   std::endl; continue;
      // }
      auto var_tail = var_sel.tail().value();
      std::string var_str = std::string(var_sel);
      // std::cerr << "    apply_jl: var: " << var_sel << std::endl;
      if (mvmap_.has_series(var_tail)) {
        // std::cerr << "    apply_jl: has series: " << var_sel << std::endl;
        auto val = mvmap_.get_as_variant(var_tail, loc);
        if (val.has_value()) {
          // std::cerr << "    apply_jl: val has value" << std::endl;
          std::visit(
              [&submission_data, &loc, &var_str](auto&& v) {
                submission_data[var_str] = boost::json::value(v);
                // std::cerr << "    apply_jl: submission_data[" << var_str
                //           << "] = " << v << " at loc " << loc << "."
                //           << std::endl;
              },
              *val);
        } else {
          std::cerr << "    apply_jl: no value for " << var_sel << std::endl;
          submission_data[var_str] = boost::json::value();
        }
      } else {
        std::cerr << "    apply_jl: no series for " << var_sel << std::endl;
      }
    }
    // std::cerr << "    apply_jl: submission_data: " << submission_data
    //           << std::endl;
    auto res = jlrule->apply(jsonlogic::json_accessor(submission_data));
    // jsonlogic::apply(
    //     *expression_rule, jsonlogic::data_accessor(submission_data));
    // std::cerr << "    apply_jl: res: " << res << std::endl;
    return jsonlogic::unpack_value<bool>(res);
  };

  return apply_jl;
}

std::vector<testgraph::node_t> where_nodes(const testgraph::testgraph& g,
                                           boost::json::object& expression) {
  std::vector<testgraph::node_t> filtered_results;
  // boost::json::object exp2(expression);

  // std::cerr << "  where: expression: " << expression << std::endl;

  auto nodemap = g.nodemap();
  boost::json::object submission_data;
  auto apply_jl = parse_where_expression(nodemap, expression, submission_data);
  nodemap.for_all([&filtered_results, &apply_jl, &nodemap,
                   &expression](const auto &key, const auto &loc) {
    // std::cerr << "  where for_all key: " << key << std::endl;
    if (apply_jl(loc)) {
      // std::cerr << "  where: applied!" << std::endl;
      filtered_results.push_back(key);
    }
  });

  return filtered_results;
}
