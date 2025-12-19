// Copyright 2020 Lawrence Livermore National Security, LLC and other CLIPPy Project Developers.
// See the top-level COPYRIGHT file for details.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <boost/json.hpp>

namespace clippy {
  class object;
  class array;
  
  struct array {    
      using json_type = ::boost::json::array;
    
      array()                        = default;
      ~array()                       = default;
      array(const array&)            = default;
      array(array&&)                 = default;
      array& operator=(const array&) = default;
      array& operator=(array&&)      = default;
      
      template <class JsonType>
      void append_json(JsonType obj)
      {
        data.emplace_back(std::move(obj).json());
      }

      template <class T>
      void append_val(T obj)
      {
        data.emplace_back(std::move(obj));
      }
      
            json_type&  json() &       { return data; }
      const json_type&  json() const & { return data; }
            json_type&& json() &&      { return std::move(data); }
                
    private:
      json_type data;
  };
    
  struct object {
      using json_type = ::boost::json::object;

      object()                         = default;
      ~object()                        = default;
      object(const object&)            = default;
      object(object&&)                 = default;
      object& operator=(const object&) = default;
      object& operator=(object&&)      = default;
      
      object(const json_type& dat)
      : data(dat)
      {}
      
      template <class JsonType>
      void set_json(const std::string& key, JsonType val)
      {
        data[key] = std::move(val).json();
      }
        
      template <class T>    
      void set_val(const std::string& key, T val)
      {
        data[key] = ::boost::json::value_from(val);
      }
    
            json_type&  json() &       { return data; }
      const json_type&  json() const & { return data; }
            json_type&& json() &&      { return std::move(data); }
                                
    private:
      json_type data;
  };
}

