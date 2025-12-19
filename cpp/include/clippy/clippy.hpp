// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy
// Project Developers. See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <clippy/version.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include "clippy-object.hpp"

// #if __has_include(<mpi.h>)
// #include <mpi.h>
// #endif

#if __has_include("clippy-log.hpp")
#include "clippy-log.hpp"
#else
static constexpr bool LOG_JSON = false;
#endif

#if WITH_YGM
#include <ygm/comm.hpp>
#endif /* WITH_YGM */

#include <boost/json/src.hpp>

namespace clippy {

namespace {
template <class T>
struct is_container {
  enum {
    value = false,
  };
};

template <class T, class Alloc>
struct is_container<std::vector<T, Alloc>> {
  enum {
    value = true,
  };
};

boost::json::value asContainer(boost::json::value val, bool requiresContainer) {
  if (!requiresContainer) return val;
  if (val.is_array()) return val;

  boost::json::array res;

  res.emplace_back(std::move(val));
  return res;
}

std::string clippyLogFile{"clippy.log"};

#if WITH_YGM
std::string userInputString;

struct BcastInput {
  void operator()(std::string inp) const {
    userInputString = std::move(inp);
    if (LOG_JSON) {
      std::ofstream logfile{clippyLogFile, std::ofstream::app};

      logfile << "--in-> " << userInputString << std::endl;
    }
  }
};
#endif
}  // namespace

class clippy {
 public:
  clippy(const std::string &name, const std::string &desc) {
    get_value(m_json_config, "method_name") = name;
    get_value(m_json_config, "desc") = desc;
    get_value(m_json_config, "version") = std::string(CLIPPY_VERSION_NAME);
  }

  /// Makes a method a member of a class \ref className and documentation \ref
  /// docString.
  // \todo Shall we also model the module name?
  //       The Python serialization module has preliminary support for modules,
  //       but this is currently not used.
  void member_of(const std::string &className, const std::string &docString) {
    get_value(m_json_config, class_name_key) = className;
    get_value(m_json_config, class_desc_key) = docString;
  }

  ~clippy() {
    const bool requiresResponse =
        !(m_json_return.is_null() && m_json_state.empty() &&
          m_json_overwrite_args.empty() && m_json_selectors.is_null());

    if (requiresResponse) {
      int rank = 0;
#ifdef MPI_VERSION
      if (::MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
      }
#endif
      if (rank == 0) {
        write_response(std::cout);

        if (LOG_JSON) {
          std::ofstream logfile{clippyLogFile, std::ofstream::app};

          logfile << "<-out- ";
          write_response(logfile);
          logfile << std::endl;
        }
      }
    }
  }

  template <class M>
  void log(std::ofstream &logfile, const M &msg) {
    if (LOG_JSON) logfile << msg << std::flush;
  }

  template <class M>
  void log(const M &msg) {
    if (!LOG_JSON) return;

    std::ofstream logfile{clippyLogFile, std::ofstream::app};
    log(logfile, msg);
  }

  template <typename T>
  void add_required(const std::string &name, const std::string &desc) {
    add_required_validator<T>(name);
    size_t position = m_next_position++;
    get_value(m_json_config, "args", name, "desc") = desc;
    get_value(m_json_config, "args", name, "position") = position;
  }

  template <typename T>
  void add_required_state(const std::string &name, const std::string &desc) {
    add_required_state_validator<T>(name);

    get_value(m_json_config, state_key, name, "desc") = desc;
  }

  template <typename T>
  void add_optional(const std::string &name, const std::string &desc,
                    const T &default_val) {
    add_optional_validator<T>(name);
    get_value(m_json_config, "args", name, "desc") = desc;
    get_value(m_json_config, "args", name, "position") = -1;
    get_value(m_json_config, "args", name, "default_val") =
        boost::json::value_from(default_val);
  }

  void update_selectors(
      const std::map<std::string, std::string> &map_selectors) {
    m_json_selectors = boost::json::value_from(map_selectors);
  }

  template <typename T>
  void returns(const std::string &desc) {
    get_value(m_json_config, returns_key, "desc") = desc;
  }

  void returns_self() {
    get_value(m_json_config, "returns_self") = true;
    m_returns_self = true;
  }

  void return_self() { m_returns_self = true; }

  template <typename T>
  void to_return(const T &value) {
    // if (detail::get_type_name<T>() !=
    //     m_json_config[returns_key]["type"].get<std::string>()) {
    //   throw std::runtime_error("clippy::to_return(value):  Invalid type.");
    // }
    m_json_return = boost::json::value_from(value);
  }

  void to_return(::clippy::object value) {
    m_json_return = std::move(value).json();
  }

  void to_return(::clippy::array value) {
    m_json_return = std::move(value).json();
  }

  bool parse(int argc, char **argv) {
    const char *JSON_FLAG = "--clippy-help";
    const char *DRYRUN_FLAG = "--clippy-validate";
    if (argc == 2 && std::string(argv[1]) == JSON_FLAG) {
      if (LOG_JSON) {
        std::ofstream logfile{clippyLogFile, std::ofstream::app};

        logfile << "<-hlp- " << m_json_config << std::endl;
      }
      std::cout << m_json_config;
      return true;
    }
    std::string buf;
    std::getline(std::cin, buf);
    m_json_input = boost::json::parse(buf);

    if (LOG_JSON) {
      std::ofstream logfile{clippyLogFile, std::ofstream::app};

      logfile << "--in-> " << m_json_input << std::endl;
    }
    validate_json_input();

    if (argc == 2 && std::string(argv[1]) == DRYRUN_FLAG) {
      return true;
    }

    // Good to go for reals
    return false;
  }

#if WITH_YGM
  bool parse(int argc, char **argv, ygm::comm &world) {
    const char *JSON_FLAG = "--clippy-help";
    const char *DRYRUN_FLAG = "--clippy-validate";

    clippyLogFile = "clippy-" + std::to_string(world.rank()) + ".log";

    if (argc == 2 && std::string(argv[1]) == JSON_FLAG) {
      if (LOG_JSON && (world.rank() == 0)) {
        std::ofstream logfile{clippyLogFile, std::ofstream::app};

        logfile << "<-hlp- " << m_json_config << std::endl;
      }

      if (world.rank0()) {
        std::cout << m_json_config;
      }
      return true;
    }

    if (world.rank() == 0) {
      std::getline(std::cin, userInputString);
      world.async_bcast(BcastInput{}, userInputString);
    }

    world.barrier();

    m_json_input = boost::json::parse(userInputString);

    if (LOG_JSON && (world.rank() == 0)) {
      std::ofstream logfile{clippyLogFile, std::ofstream::app};

      logfile << "--in-> " << m_json_input << std::endl;
    }
    validate_json_input();

    if (argc == 2 && std::string(argv[1]) == DRYRUN_FLAG) {
      return true;
    }

    // Good to go for reals
    return false;
  }
#endif /* WITH_YGM */

  template <typename T>
  T get(const std::string &name) {
    static constexpr bool requires_container = is_container<T>::value;

    if (has_argument(name)) {  // if the argument exists
      auto foo = get_value(m_json_input, name);
      return boost::json::value_to<T>(
          asContainer(get_value(m_json_input, name), requires_container));
    } else {  // it's an optional
      // std::cerr << "optional argument found: " + name << std::endl;
      return boost::json::value_to<T>(
          asContainer(get_value(m_json_config, "args", name, "default_val"),
                      requires_container));
    }
  }

  bool has_state(const std::string &name) const {
    return has_value(m_json_input, state_key, name);
  }

  template <typename T>
  T get_state(const std::string &name) const {
    return boost::json::value_to<T>(get_value(m_json_input, state_key, name));
  }

  template <typename T>
  void set_state(const std::string &name, T val) {
    // if no state exists (= empty), then copy it from m_json_input if it exists
    // there;
    //   otherwise just start with an empty state.
    if (m_json_state.empty())
      if (boost::json::value *stateValue =
              m_json_input.get_object().if_contains(state_key))
        m_json_state = stateValue->as_object();

    m_json_state[name] = boost::json::value_from(val);
  }

  template <typename T>
  void overwrite_arg(const std::string &name, const T &val) {
    m_json_overwrite_args[name] = boost::json::value_from(val);
  }

  bool has_argument(const std::string &name) const {
    return m_json_input.get_object().contains(name);
  }

  bool is_class_member_function() const try {
    return m_json_config.get_object().if_contains(class_name_key) != nullptr;
  } catch (const std::invalid_argument &) {
    return false;
  }

 private:
  void write_response(std::ostream &os) const {
    // construct the response object
    boost::json::object json_response;

    // incl. the response if it has been set
    if (m_returns_self) {
      json_response["returns_self"] = true;
    } else if (!m_json_return.is_null())
      json_response[returns_key] = m_json_return;

    // only communicate the state if it has been explicitly set.
    //   no state -> no state update
    if (!m_json_state.empty()) json_response[state_key] = m_json_state;

    if (!m_json_selectors.is_null())
      json_response[selectors_key] = m_json_selectors;

    // only communicate the pass by reference arguments if explicitly set
    if (!m_json_overwrite_args.empty())
      json_response["references"] = m_json_overwrite_args;

    // write the response
    os << json_response << std::endl;
  }

  void validate_json_input() {
    for (auto &kv : m_input_validators) {
      kv.second(m_json_input);
    }
    // TODO: Warn/Check for unknown args
  }

  template <typename T>
  void add_optional_validator(const std::string &name) {
    if (m_input_validators.count(name) > 0) {
      std::stringstream ss;
      ss << "CLIPPy ERROR:   Cannot have duplicate argument names: " << name
         << "\n";
      throw std::runtime_error(ss.str());
    }
    m_input_validators[name] = [name](const boost::json::value &j) {
      if (!j.get_object().contains(name)) {
        return;
      }  // Optional, only eval if present
      try {
        static constexpr bool requires_container = is_container<T>::value;

        boost::json::value_to<T>(
            asContainer(get_value(j, name), requires_container));
      } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "CLIPPy ERROR:  Optional argument " << name << ": \"" << e.what()
           << "\"\n";
        throw std::runtime_error(ss.str());
      }
    };
  }

  template <typename T>
  void add_required_validator(const std::string &name) {
    if (m_input_validators.count(name) > 0) {
      throw std::runtime_error("Clippy:: Cannot have duplicate argument names");
    }
    m_input_validators[name] = [name](const boost::json::value &j) {
      if (!j.get_object().contains(name)) {
        std::stringstream ss;
        ss << "CLIPPy ERROR:  Required argument " << name << " missing.\n";
        throw std::runtime_error(ss.str());
      }
      try {
        static constexpr bool requires_container = is_container<T>::value;

        boost::json::value_to<T>(
            asContainer(get_value(j, name), requires_container));
      } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "CLIPPy ERROR:  Required argument " << name << ": \"" << e.what()
           << "\"\n";
        throw std::runtime_error(ss.str());
      }
    };
  }

  template <typename T>
  void add_required_state_validator(const std::string &name) {
    // state validator keys are prefixed with "state::"
    std::string key{state_key};

    key += "::";
    key += name;

    if (m_input_validators.count(key) > 0) {
      throw std::runtime_error("Clippy:: Cannot have duplicate state names");
    }

    auto state_validator = [name](const boost::json::value &j) {
      // \todo check that the path j["state"][name] exists
      try {
        // try access path and value conversion
        boost::json::value_to<T>(
            j.as_object().at(clippy::state_key).as_object().at(name));
        //~ boost::json::value_to<T>(get_value(j, clippy::state_key, name));
      } catch (const std::exception &e) {
        std::stringstream ss;
        ss << "CLIPPy ERROR: state attribute " << name << ": \"" << e.what()
           << "\"\n";
        throw std::runtime_error(ss.str());
      }
    };

    m_input_validators[key] = state_validator;
  }

  static constexpr bool has_value(const boost::json::value &) { return true; }

  template <typename... argts>
  static bool has_value(const boost::json::value &value, const std::string &key,
                        const argts &...inner_keys) {
    if (const boost::json::object *obj = value.if_object())
      if (const auto pos = obj->find(key); pos != obj->end())
        return has_value(pos->value(), inner_keys...);

    return false;
  }

  static boost::json::value &get_value(boost::json::value &value,
                                       const std::string &key) {
    if (!value.is_object()) {
      value.emplace_object();
    }
    return value.get_object()[key];
  }

  template <typename... argts>
  static boost::json::value &get_value(boost::json::value &value,
                                       const std::string &key,
                                       const argts &...inner_keys) {
    if (!value.is_object()) {
      value.emplace_object();
    }
    return get_value(value.get_object()[key], inner_keys...);
  }

  static const boost::json::value &get_value(const boost::json::value &value,
                                             const std::string &key) {
    return value.get_object().at(key);
  }

  template <typename... argts>
  static const boost::json::value &get_value(const boost::json::value &value,
                                             const std::string &key,
                                             const argts &...inner_keys) {
    return get_value(value.get_object().at(key), inner_keys...);
  }

  boost::json::value m_json_config;
  boost::json::value m_json_input;
  boost::json::value m_json_return;
  boost::json::value m_json_selectors;
  boost::json::object m_json_state;
  boost::json::object m_json_overwrite_args;
  bool m_returns_self = false;

  boost::json::object *m_json_input_state = nullptr;
  size_t m_next_position = 0;

  std::map<std::string, std::function<void(const boost::json::value &)>>
      m_input_validators;

 public:
  static constexpr const char *const state_key = "_state";
  static constexpr const char *const selectors_key = "_selectors";
  static constexpr const char *const returns_key = "returns";
  static constexpr const char *const class_name_key = "class_name";
  static constexpr const char *const class_desc_key = "class_desc";
};

}  // namespace clippy

namespace boost::json {
void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                const std::vector<std::pair<int, int>> &value) {
  auto &outer_array = jv.emplace_array();
  outer_array.resize(value.size());

  for (std::size_t i = 0; i < value.size(); ++i) {
    auto &inner_array = outer_array[i].emplace_array();
    inner_array.resize(2);
    inner_array[0] = value[i].first;
    inner_array[1] = value[i].second;
  }
}

std::vector<std::pair<int, int>> tag_invoke(
    boost::json::value_to_tag<std::vector<std::pair<int, int>>>,
    const boost::json::value &jv) {
  std::vector<std::pair<int, int>> value;

  auto &outer_array = jv.get_array();
  for (const auto &inner_value : outer_array) {
    const auto &inner_array = inner_value.get_array();
    value.emplace_back(
        std::make_pair(inner_array[0].as_int64(), inner_array[1].as_int64()));
  }

  return value;
}

void tag_invoke(boost::json::value_from_tag, boost::json::value &jv,
                const std::vector<std::pair<std::string, std::string>> &value) {
  auto &outer_array = jv.emplace_array();
  outer_array.resize(value.size());

  for (std::size_t i = 0; i < value.size(); ++i) {
    auto &inner_array = outer_array[i].emplace_array();
    inner_array.resize(2);
    inner_array[0] = value[i].first;
    inner_array[1] = value[i].second;
  }
}

std::vector<std::pair<std::string, std::string>> tag_invoke(
    boost::json::value_to_tag<std::vector<std::pair<std::string, std::string>>>,
    const boost::json::value &jv) {
  std::vector<std::pair<std::string, std::string>> value;

  auto &outer_array = jv.get_array();
  for (const auto &inner_value : outer_array) {
    const auto &inner_array = inner_value.get_array();
    value.emplace_back(
        std::make_pair(std::string(inner_array[0].as_string().c_str()),
                       std::string(inner_array[1].as_string().c_str())));
  }

  return value;
}
}  // namespace boost::json
