#pragma once
#include <boost/json.hpp>
// #include <boost/json/conversion.hpp>
#include <boost/json/src.hpp>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p) {
  os << "(" << p.first << ", " << p.second << ")";
  return os;
}

namespace mvmap {
using index = uint64_t;

class locator {
  static const index INVALID_LOC = std::numeric_limits<index>::max();
  index loc;
  locator(index loc) : loc(loc) {};

 public:
  template <typename K, typename... Vs>
  friend class mvmap;
  friend std::ostream &operator<<(std::ostream &os, const locator &l) {
    if (l.is_valid()) {
      os << "locator: " << l.loc;
    } else {
      os << "locator: invalid";
    }
    return os;
  }
  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, locator l);
  friend locator tag_invoke(boost::json::value_to_tag<locator> /*unused*/,
                            const boost::json::value &v);
  locator() : loc(INVALID_LOC) {};
  [[nodiscard]] bool is_valid() const { return loc != INVALID_LOC; }

  void print() const {
    if (is_valid()) {
      std::cout << "locator: " << loc << std::endl;
    } else {
      std::cout << "locator: invalid" << std::endl;
    }
  }
};
void tag_invoke(boost::json::value_from_tag /*unused*/, boost::json::value &v,
                locator l) {
  v = l.loc;
}

locator tag_invoke(boost::json::value_to_tag<locator> /*unused*/,
                   const boost::json::value &v) {
  return boost::json::value_to<index>(v);
}
template <typename K, typename... Vs>
class mvmap {
  template <typename T>
  using series = std::map<index, T>;
  using key_to_idx = std::map<K, index>;
  using idx_to_key = std::map<index, K>;
  using variants = std::variant<Vs...>;

  template <typename V>
  static void print_series(const series<V> &s) {
    for (auto el : s) {
      std::cout << el.first << " -> " << el.second << std::endl;
    }
  }

  // A locator is an opaque handle to a key in a series.

  idx_to_key itk;
  key_to_idx kti;
  std::map<std::string, std::variant<series<Vs>...>> data;
  std::map<std::string, std::string> series_desc;

 public:
  // A series_proxy is a reference to a series in an mvmap.
  template <typename V>
  class series_proxy {
    std::string m_id;
    std::string m_desc;
    key_to_idx &kti_r;
    idx_to_key &itk_r;
    series<V> &series_r;

    using series_type = V;

    // returns true if there is an index assigned to a given key
    bool has_idx_at_key(K k) const { return kti_r.contains(k); }
    // returns true if there is a key assigned to a given locator
    bool has_key_at_index(locator l) const { return itk_r.contains(l.loc); }

    // returns or creates the index for a key.
    index get_idx(K k) {
      if (!has_idx_at_key(k)) {
        index i{kti_r.size()};
        kti_r[k] = i;
        itk_r[i] = k;
        return i;
      }
      return kti_r[k];
    }

   public:
    series_proxy(std::string id, series<V> &ser, mvmap<K, Vs...> &m)
        : m_id(std::move(id)), kti_r(m.kti), itk_r(m.itk), series_r(ser) {}

    series_proxy(std::string id, const std::string &desc, series<V> &ser,
                 mvmap<K, Vs...> &m)
        : m_id(std::move(id)),
          m_desc(desc),
          kti_r(m.kti),
          itk_r(m.itk),
          series_r(ser) {}

    bool is_string_v() const { return std::is_same_v<V, std::string>; }
    bool is_double_v() const { return std::is_same_v<V, double>; }
    bool is_int64_t_v() const { return std::is_same_v<V, int64_t>; }
    bool is_bool_v() const { return std::is_same_v<V, bool>; }

    std::string id() const { return m_id; }
    std::string desc() const { return m_desc; }
    V &operator[](K k) { return series_r[get_idx(k)]; }
    const V &operator[](K k) const { return series_r[get_idx(k)]; }

    // this assumes the key exists.
    V &operator[](locator l) { return series_r[l.loc]; }
    const V &operator[](locator l) const { return series_r[l.loc]; }

    std::optional<std::reference_wrapper<V>> at(locator l) {
      if (!has_key_at_index(l) || !series_r.contains(l.loc)) {
        return std::nullopt;
      }
      return series_r[l.loc];
    };
    std::optional<std::reference_wrapper<const V>> at(locator l) const {
      if (!has_key_at_index(l) || !series_r.contains(l.loc)) {
        return std::nullopt;
      }
      return series_r[l.loc];
    };

    std::optional<std::reference_wrapper<V>> at(K k) {
      if (!has_idx_at_key(k) || !series_r.contains(get_idx(k))) {
        return std::nullopt;
      }
      return series_r[get_idx(k)];
    };

    std::optional<std::reference_wrapper<const V>> at(K k) const {
      if (!has_idx_at_key(k) || !series_r.contains(get_idx(k))) {
        return std::nullopt;
      }
      return series_r[get_idx(k)];
    };

    // this will create the key/index if it doesn't exist.
    locator get_loc(K k) { return locator(get_idx(k)); }

    // F takes (K key, locator, V value)
    template <typename F>
    void for_all(F f) {
      for (auto el : series_r) {
        f(itk_r[el.first], locator(el.first), el.second);
      }
    };

    template <typename F>
    void for_all(F f) const {
      for (auto el : series_r) {
        f(itk_r[el.first], locator(el.first), el.second);
      }
    };

    // F takes (K key, locator, V value)
    template <typename F>
    void remove_if(F f) {
      auto indices_to_delete = std::vector<index>{};
      for (auto el : series_r) {
        if (f(itk_r[el.first], locator(el.first), el.second)) {
          indices_to_delete.emplace_back(el.first);
        }
      }

      for (auto ltd : indices_to_delete) {
        erase(locator(ltd));
      }
    };

    void erase(const locator &l) {
      auto i = l.loc;
      kti_r.erase(itk_r[i]);
      itk_r.erase(i);
      series_r.erase(i);
    }

    // if the key doesn't exist, do nothing.
    void erase(const K &k) {
      if (!has_idx_at_key(k)) {
        return;
      }
      auto i = kti_r[k];
      erase(locator(i));
    }

    // this returns the key for a given locator in a series, or nullopt if the
    // locator is invalid.
    std::optional<std::reference_wrapper<const K>> get_key(
        const locator &l) const {
      if (!has_key_at_index(l.loc)) {
        return std::nullopt;
      }
      return itk_r[l.loc];
    }

    std::pair<std::optional<std::tuple<V, K, locator>>,
              std::optional<std::tuple<V, K, locator>>>
    extrema() {
      V min = std::numeric_limits<V>::max();
      V max = std::numeric_limits<V>::min();
      bool found_min = false;
      bool found_max = false;
      locator min_loc;
      locator max_loc;
      K min_key;
      K max_key;
      for_all([&min, &max, &found_min, &found_max, &min_loc, &max_loc, &min_key,
               &max_key](auto k, auto l, auto v) {
        if (v < min) {
          min = v;
          min_loc = l;
          min_key = k;
          found_min = true;
        }
        if (v > max) {
          max = v;
          max_loc = l;
          max_key = k;
          found_max = true;
        }
      });
      std::optional<std::tuple<V, K, locator>> min_opt, max_opt;
      if (found_min) {
        min_opt = std::make_tuple(min, min_key, min_loc);
      } else {
        min_opt = std::nullopt;
      }
      if (found_max) {
        max_opt = std::make_tuple(max, max_key, max_loc);
      } else {
        max_opt = std::nullopt;
      }
      return std::make_pair(min_opt, max_opt);
    }

    std::map<V, size_t> count() {
      std::map<V, size_t> ct;
      for_all([&ct](auto /*unused*/, auto /*unused*/, auto v) { ct[v]++; });
      return ct;
    }

    void print() {
      std::cout << "id: " << m_id << ", ";
      std::cout << "desc: " << m_desc << ", ";
      std::string dtype = "unknown";
      if (is_string_v()) {
        dtype = "string";
      } else if (is_double_v()) {
        dtype = "double";
      } else if (is_int64_t_v()) {
        dtype = "int64_t";
      } else if (is_bool_v()) {
        dtype = "bool";
      }
      std::cout << "dtype: " << dtype << ", ";
      // std::cout << "kti_r.size(): " << kti_r.size() << std::endl;
      // std::cout << "itk_r.size(): " << itk_r.size() << std::endl;
      std::cout << series_r.size() << " entries" << std::endl;
      // std::cout << "elements: " << std::endl;
      for (auto el : series_r) {
        std::cout << "  " << itk_r[el.first] << " -> " << el.second
                  << std::endl;
      }
    }

  };  // end of series
  /////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////
  mvmap(const idx_to_key &itk, const key_to_idx &kti,
        const std::map<std::string, std::variant<series<Vs>...>> &data)
      : itk(itk), kti(kti), data(data) {}

  mvmap() = default;
  friend void tag_invoke(boost::json::value_from_tag /*unused*/,
                         boost::json::value &v, const mvmap<K, Vs...> &m) {
    v = {{"itk", boost::json::value_from(m.itk)},
         {"kti", boost::json::value_from(m.kti)},
         {"data", boost::json::value_from(m.data)}};
  }

  friend mvmap<K, Vs...> tag_invoke(
      boost::json::value_to_tag<mvmap<K, Vs...>> /*unused*/,
      const boost::json::value &v) {
    const auto &obj = v.as_object();
    using index = uint64_t;
    // template <typename T> using series = std::map<index, T>;
    using key_to_idx = std::map<K, index>;
    using idx_to_key = std::map<index, K>;
    return {boost::json::value_to<idx_to_key>(obj.at("itk")),
            boost::json::value_to<key_to_idx>(obj.at("kti")),
            boost::json::value_to<
                std::map<std::string, std::variant<series<Vs>...>>>(
                obj.at("data"))};
  }

  [[nodiscard]] size_t size() const { return kti.size(); }
  bool add_key(const K &k) {
    if (kti.count(k) > 0) {
      return false;
    }
    auto i = kti.size();
    kti[k] = i;
    itk[i] = k;
    return true;
  }

  [[nodiscard]] std::vector<std::pair<std::string, std::string>> list_series() {
    std::vector<std::pair<std::string, std::string>> ser_pairs;
    for (auto el : series_desc) {
      ser_pairs.push_back(el);
    }
    return ser_pairs;
  }

  [[nodiscard]] bool has_series(const std::string &id) const {
    return data.contains(id);
  }

  template <typename V>
  [[nodiscard]] bool has_series(const std::string &id) const {
    // std::cerr << "has_series: " << id << std::endl;
    // std::cerr << "V = " << typeid(V).name() << std::endl;
    // std::cerr << "data.contains(id): " << data.contains(id) << std::endl;
    // if (data.contains(id)) {
    //   std::cerr << "std::holds_alternative<series<V>>(data.at(id)): "
    //             << std::holds_alternative<series<V>>(data.at(id)) <<
    //             std::endl;
    // }
    return data.contains(id) && std::holds_alternative<series<V>>(data.at(id));
  }
  bool contains(const K &k) { return kti.contains(k); }
  auto keys() const { return std::views::keys(kti); }

  void add_row(const K &key,
               const std::map<std::string, std::variant<Vs...>> &row) {
    auto loc = locator(kti.size());
    for (const auto &el : row) {
      if (!has_series(el.first)) {
        continue;
      }
      std::visit(
          [&el, &key, this](auto &ser) {
            std::cout << "adding to series " << el.first << std::endl;
            using variant_type = std::decay_t<decltype(ser.begin()->second)>;
            auto sproxy = get_series<std::decay_t<variant_type>>(el.first)
                              .value();  // this is a series_proxy
            // TODO: make sure this actually sets itk and kti correctly.
            sproxy[key] = std::get<variant_type>(el.second);
          },
          data[el.first]);
    }
  }

  void rem_row(const K &key) {
    auto index = kti[key];
    for (auto &el : data) {
      std::visit([&key, &index, this](auto &ser) { ser.erase(index); },
                 el.second);
    }

    kti.erase(key);
    itk.erase(index);
  }
  // adds a new column (series) to the mvmap and returns true. If already
  // exists, return false
  template <typename V>
  std::optional<series_proxy<V>> add_series(const std::string &sel,
                                            const std::string &desc = "") {
    if (has_series(sel)) {
      return std::nullopt;
    }
    data[sel] = series<V>{};
    series_desc[sel] = desc;
    return series_proxy(sel, desc, std::get<series<V>>(data[sel]), *this);
  }

  // copies an existing column (series) to a new (unmanifested) column and
  // returns true. If the new column already exists, or if the existing column
  // doesn't, return false.
  bool copy_series(const std::string &from, const std::string &to,
                   const std::optional<std::string> &desc = std::nullopt) {
    if (has_series(to) || !has_series(from)) {
      std::cerr << "copy_series failed from " << from << " to " << to
                << std::endl;
      return false;
    }
    // std::cerr << "copying series from " << from << " to " << to << std::endl;
    data[to] = data[from];
    series_desc[to] = desc.has_value() ? desc.value() : series_desc[from];
    return true;
  }

  template <typename V>
  std::optional<series_proxy<V>> get_series(const std::string &sel) {
    if (!has_series<V>(sel)) {
      // series doesn't exist or is of the wrong type.
      return std::nullopt;
    }
    // return series_proxy<V>(sel, this->series_desc.at(sel),
    //                        std::get<series<V>>(data[sel]), *this);
    auto foo = series_desc[sel];
    return series_proxy<V>(sel, foo, std::get<series<V>>(data[sel]), *this);
  }

  bool series_is_string(const std::string &sel) const {
    return has_series<std::string>(sel);
  }

  bool series_is_double(const std::string &sel) const {
    return has_series<double>(sel);
  }

  bool series_is_int64_t(const std::string &sel) const {
    return has_series<int64_t>(sel);
  }

  bool series_is_bool(const std::string &sel) const {
    return has_series<bool>(sel);
  }

  // std::optional<series_proxy<variants>> get_variant_series(
  //     const std::string &sel) {
  //   if (!has_series(sel)) {
  //     return std::nullopt;
  //   }

  //   using vtype = decltype(data[sel]);
  //   return series_proxy<vtype>(sel, this->series_desc.at(sel),
  //   data.at(sel),
  //                              this);
  // }

  void drop_series(const std::string &sel) {
    if (!has_series(sel)) {
      return;
    }
    data.erase(sel);
    series_desc.erase(sel);
  }

  std::optional<mvmap::variants> get_as_variant(const std::string &sel,
                                                const locator &loc) {
    auto col = data[sel];
    std::optional<mvmap::variants> val;
    std::visit(
        [&val, sel, loc, this](auto &ser) {
          using T = std::decay_t<decltype(ser.begin()->second)>;
          auto sproxy = get_series<std::decay_t<T>>(sel)
                            .value();  // this is a series_proxy
          val = sproxy.at(loc);
        },
        col);
    return val;
    // return data[sel][loc.loc];
  }

  std::optional<mvmap::variants> get_as_variant(const std::string &sel,
                                                const K &key) {
    auto col = data[sel];
    std::optional<mvmap::variants> val;
    std::visit(
        [&val, sel, key, this](auto &ser) {
          using T = std::decay_t<decltype(ser.begin()->second)>;
          auto sproxy = get_series<std::decay_t<T>>(sel)
                            .value();  // this is a series_proxy
          val = sproxy.at(key);
        },
        col);
    return val;
  }

  // F is a function that takes a key and a locator.
  // Users will need to close over series_proxies that they want to use.
  template <typename F>
  void for_all(F f) {
    for (auto &idx : kti) {
      f(idx.first, locator(idx.second));
    }
  }

  template <typename F>
  void remove_if(F f) {
    std::vector<index> indices_to_delete;
    for (auto &idx : kti) {
      if (f(idx.first, locator(idx.second))) {
        indices_to_delete.emplace_back(idx.second);
      }
    }

    for (auto &idx : indices_to_delete) {
      kti.erase(itk[idx]);
      itk.erase(idx);
      for (auto &id_ser : data) {
        std::visit([&idx](auto &ser) { ser.erase(idx); }, id_ser.second);
      }
    }
  }

  void print() {
    std::cout << "mvmap with " << data.size() << " series: " << std::endl;
    for (auto &el : data) {
      std::cout << "series " << el.first << ":" << std::endl;
      std::visit(
          [&el, this](auto &ser) {
            using T = std::decay_t<decltype(ser.begin()->second)>;
            auto sproxy = get_series<std::decay_t<T>>(el.first)
                              .value();  // this is a series_proxy
            sproxy.print();
          },
          el.second);

      // std::cout << "    second: " << el.second << std::endl;
      // auto foo = get_variant_series(el.first).value();
      // foo.visit([](auto &ser) { print_series(ser); });
      // print_series(el.second);
    }
  }

  std::string str_cols(const std::vector<std::string> &cols) {
    std::stringstream sstr;
    for_all([this, &cols, &sstr](auto key, auto loc) {
      sstr << key << "@" << loc.loc;
      for (auto &col : cols) {
        auto v = get_as_variant(col, loc);
        if (v.has_value()) {
          std::visit(
              [&col, &sstr](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) {
                  sstr << " (str) " << col << ": " << arg;
                } else if constexpr (std::is_same_v<T, double>) {
                  sstr << " (dbl) " << col << ": " << arg;
                } else if constexpr (std::is_same_v<T, int64_t>) {
                  sstr << " (int) " << col << ": " << arg;
                } else if constexpr (std::is_same_v<T, bool>) {
                  sstr << " (bool) " << col << ": " << arg;
                }
              },
              v.value());
        }
      }
      sstr << std::endl;
    });
    return sstr.str();
  }

  std::map<std::string, mvmap::variants> get_series_vals_at(
      const K &key, const std::vector<std::string> &cols) {
    std::vector<std::variant<series_proxy<Vs>...>> proxies;

    for (auto &col : cols) {
      if (has_series(col)) {
        std::visit(
            [this, &col, &proxies](auto &coldata) {
              using T = std::decay_t<decltype(coldata)>::mapped_type;
              auto sproxy =
                  get_series<T>(col).value();  // this is a series_proxy
              proxies.push_back(sproxy);
            },
            data[col]);
      }
    }

    std::map<std::string, mvmap::variants> row;
    for (auto &sproxy : proxies) {
      std::visit(
          [&row, &key](auto &&arg) {
            auto v = arg.at(key);
            if (v.has_value()) {
              row[arg.id()] = arg[key];
            }
          },
          sproxy);
    }
    return row;
  }
};
};  // namespace mvmap
