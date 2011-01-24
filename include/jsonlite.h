/*
The MIT License

Copyright (c) 2011 lyo.kato@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _JSONLITE_H_
#define _JSONLITE_H_
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <boost/variant.hpp>

#define IS_XDIGIT(u) \
    (((u) >= '0' && (u) <= '9') || ((u) >= 'a' && (u) <= 'f') || ((u) >= 'A' && (u) <= 'F'))

namespace jsonlite {

// escape invalid '\' and '"' included in string
std::string 
escape_json_string(const std::string& s)
{
  std::ostringstream os;
  const char *c = s.c_str();
  std::size_t rest = s.size();
  char u1, u2, u3, u4;

  while (*c != '\0') {
    if (*c == '\\') {
      if (!(rest > 1)) {
        os << "\\\\";
        break;
      }
      ++c; --rest;
      if (*c == '"' || *c == '/' || *c == '\\' || *c == 'b'
       || *c == 'n' || *c == 'r' || *c == 't') {
        // OK
        os << '\\' << *c;
        ++c; --rest;
        continue;
      }
      else if (*c == 'u') {
        // check next 4 digit
        if (rest > 4) {
                ++c; --rest;
          u1 = *c++; --rest; 
          u2 = *c++; --rest; 
          u3 = *c++; --rest;
          u4 = *c++; --rest;
          if (IS_XDIGIT(u1) && IS_XDIGIT(u2) && IS_XDIGIT(u3) && IS_XDIGIT(u4)) {
            // OK
            os << "\\u" << u1 << u2 << u3 << u4;
            continue;
          } else {
            os << "\\\\u";
            // rollback
            --c; ++rest;
            --c; ++rest;
            --c; ++rest;
            --c; ++rest;
            continue;
          }
        } else {
          os << "\\\\";
          continue;
        }
      }
      else {
        os << "\\\\";
        continue;
      }
    }
    else if (*c == '"') {
      os << "\\\"";
      ++c; --rest;
      continue;
    }
    else {
      os << *c;
      ++c; --rest;
      continue;
    }
  }
  return os.str();
}


class json_null;
class json_bool;
class json_array;
class json_object;
class json_string;
class json_visitor;

typedef boost::variant<
    boost::recursive_wrapper<json_null>, 
    boost::recursive_wrapper<json_object>, 
    boost::recursive_wrapper<json_array>, 
    boost::recursive_wrapper<json_string>, 
    boost::recursive_wrapper<json_bool>
> json_value;

typedef std::pair<std::string, json_value> json_kv;
typedef std::map<std::string, json_value> json_kv_map;

std::string to_json(const json_object& obj);
std::string to_json(const json_array& array);

struct json_null { };

class json_string {
 public:
  json_string(const std::string& s) : str_(s) { }
  json_string(const char* s) : str_(s) { }
  std::string str() const { return escape_json_string(str_); }
 private:
  std::string str_;
};

class json_bool {
 public:
  json_bool(bool b) : b_(b) { }
  bool operator()() const { return b_; };
 private:
  bool b_;
};

class json_array {
  friend class json_visitor;
  friend std::ostream& operator<<(std::ostream& os, const json_array& array);
 public:
  json_array() { }
  json_array(const json_value& value) {
    children_.push_back(value);
  }
  json_array(bool b) {
    children_.push_back(json_bool(b));
  }
  json_array(const std::string& s) {
    children_.push_back(json_string(s));
  }
  json_array(const char* s) {
    children_.push_back(json_string(s));
  }
  json_array& operator()(const json_value& value) {
    children_.push_back(value);
    return *this;
  }
  json_array& operator()(bool b) {
    children_.push_back(json_bool(b));
    return *this;
  }
  json_array& operator()(const std::string& s) {
    children_.push_back(json_string(s));
    return *this;
  }
  json_array& operator()(const char* s) {
    children_.push_back(json_string(s));
    return *this;
  }
  std::string str() const { return to_json(*this); }
private:
  std::vector<json_value> children_;
};

class json_object {
  friend class json_visitor;
  friend std::ostream& operator<<(std::ostream& os, const json_object& obj);
 public:
  json_object() { }
  json_object(const std::string& key, const json_value& value) {
    children_[key] = value;
  }
  json_object(const std::string& key, bool b) {
    children_[key] = json_bool(b);
  }
  json_object(const std::string& key, const std::string& s) {
    children_[key] = json_string(s);
  }
  json_object(const std::string& key, const char* s) {
    children_[key] = json_string(s);
  }
  json_object& operator()(const std::string& key, const json_value& value) {
    children_[key] = value;
    return *this;
  }
  json_object& operator()(const std::string& key, bool b) {
    children_[key] = json_bool(b);
    return *this;
  }
  json_object& operator()(const std::string& key, const std::string& s) {
    children_[key] = json_string(s);
    return *this;
  }
  json_object& operator()(const std::string& key, const char* s) {
    children_[key] = json_string(s);
    return *this;
  }
  std::string str() const { return to_json(*this); }
private:
  json_kv_map children_;
};

class json_visitor : public boost::static_visitor<std::string> {
 public:
  std::string operator()(const json_object& t) const {
    std::ostringstream os;
    json_value value;
    os << "{";
    json_kv_map::const_iterator iter = t.children_.begin();
    while (true) {
      os << "\"" << escape_json_string(iter->first) << "\":";
      value = iter->second;
      os << boost::apply_visitor(*this, value);
      ++iter;
      if (iter != t.children_.end())
        os << ",";
      else
        break;
    }
    os << "}";
    return os.str();
  }
  std::string operator()(const json_array& t) const {
    std::ostringstream os;
    json_value value;
    os << "[";
    std::vector<json_value>::const_iterator iter = t.children_.begin();
    while (true) {
      value = *iter;
      os << boost::apply_visitor(*this, value);
      ++iter;
      if (iter != t.children_.end())
        os << ",";
      else
        break;
    }
    os << "]";
    return os.str();
  }
  std::string operator()(const json_null&   t) const { return "null";                 }
  std::string operator()(const json_string& s) const { return "\"" + s.str() + "\"";  }
  std::string operator()(const json_bool&   b) const { return b() ? "true" : "false"; }
};

std::string
to_json(const json_object& obj) {
  json_value value(obj);
  return boost::apply_visitor(json_visitor(), value);
}

std::string
to_json(const json_array& array) {
  json_value value(array);
  return boost::apply_visitor(json_visitor(), value);
}

std::ostream&
operator<<(std::ostream& os, const json_object& obj)
{
  os << obj.str();
  return os;
}

std::ostream&
operator<<(std::ostream& os, const json_array& array)
{
  os << array.str();
  return os;
}

}  // end of namespace

#endif
