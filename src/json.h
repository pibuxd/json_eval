#ifndef JSON_H
#define JSON_H

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <exception>

class JSONError : public std::exception {
    std::string message;
public:
    JSONError(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class JSONValue;

using JSONObject = std::map<std::string, JSONValue>;
using JSONArray = std::vector<JSONValue>;

class JSONValue {
    std::variant<std::nullptr_t, bool, double, std::string, JSONObject, JSONArray> value;
public:
    JSONValue() : value(nullptr) {}
    JSONValue(std::nullptr_t) : value(nullptr) {}
    JSONValue(bool b) : value(b) {}
    JSONValue(double d) : value(d) {}
    JSONValue(const std::string& s) : value(s) {}
    JSONValue(const char* s) : value(std::string(s)) {}
    JSONValue(const JSONObject& o) : value(o) {}
    JSONValue(const JSONArray& a) : value(a) {}

    bool is_null() const { return std::holds_alternative<std::nullptr_t>(value); }
    bool is_bool() const { return std::holds_alternative<bool>(value); }
    bool is_number() const { return std::holds_alternative<double>(value); }
    bool is_string() const { return std::holds_alternative<std::string>(value); }
    bool is_object() const { return std::holds_alternative<JSONObject>(value); }
    bool is_array() const { return std::holds_alternative<JSONArray>(value); }

    bool as_bool() const;
    double as_number() const;
    const std::string& as_string() const;
    const JSONObject& as_object() const;
    const JSONArray& as_array() const;

    std::string to_string() const;

    friend class JSON;
};

class JSON {
    const std::string& text;
    size_t index;

    char peek() const;
    char get();
    void skip_whitespace();

public:
    JSON(const std::string& text) : text(text), index(0) {}
    static JSONValue parse(const std::string& text);
    JSONValue parse_value();
    JSONValue parse_null();
    JSONValue parse_bool();
    JSONValue parse_number();
    JSONValue parse_string();
    JSONValue parse_array();
    JSONValue parse_object();
};


#endif

