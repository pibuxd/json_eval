#include "json.h"
#include <cctype>
#include <sstream>

bool JSONValue::as_bool() const {
    if (is_bool()) return std::get<bool>(value);
    throw JSONError("Value is not a boolean");
}

double JSONValue::as_number() const {
    if (is_number()) return std::get<double>(value);
    throw JSONError("Value is not a number");
}

const std::string& JSONValue::as_string() const {
    if (is_string()) return std::get<std::string>(value);
    throw JSONError("Value is not a string");
}

const JSONObject& JSONValue::as_object() const {
    if (is_object()) return std::get<JSONObject>(value);
    throw JSONError("Value is not an object");
}

const JSONArray& JSONValue::as_array() const {
    if (is_array()) return std::get<JSONArray>(value);
    throw JSONError("Value is not an array");
}

std::string JSONValue::to_string() const {
    if (is_null()) return "null";
    if (is_bool()) return as_bool() ? "true" : "false";
    if (is_number()) {
        std::ostringstream oss;
        oss << as_number();
        return oss.str();
    }
    if (is_string()) return as_string();
    if (is_array()) {
        const JSONArray& arr = as_array();
        std::string s = "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            s += arr[i].to_string();
            if (i != arr.size() - 1) s += ", ";
        }
        s += "]";
        return s;
    }
    if (is_object()) {
        const JSONObject& obj = as_object();
        std::string s = "{";
        size_t i = 0;
        for (const auto& pair : obj) {
            s += "\"" + pair.first + "\": " + pair.second.to_string();
            if (i != obj.size() - 1) s += ", ";
            ++i;
        }
        s += "}";
        return s;
    }
    return "";
}

char JSON::peek() const {
    if (index < text.size()) return text[index];
    return '\0';
}

char JSON::get() {
    if (index < text.size()) return text[index++];
    return '\0';
}

void JSON::skip_whitespace() {
    while (std::isspace(peek())) get();
}

JSONValue JSON::parse_value() {
    skip_whitespace();
    char c = peek();
    if (c == 'n') return parse_null();
    if (c == 't' || c == 'f') return parse_bool();
    if (c == '-' || std::isdigit(c)) return parse_number();
    if (c == '"') return parse_string();
    if (c == '[') return parse_array();
    if (c == '{') return parse_object();
    throw JSONError("Invalid JSON value");
}

JSONValue JSON::parse_null() {
    if (text.substr(index, 4) == "null") {
        index += 4;
        return JSONValue(nullptr);
    }
    throw JSONError("Invalid JSON null");
}

JSONValue JSON::parse_bool() {
    if (text.substr(index, 4) == "true") {
        index += 4;
        return JSONValue(true);
    } else if (text.substr(index, 5) == "false") {
        index += 5;
        return JSONValue(false);
    }
    throw JSONError("Invalid JSON boolean");
}

JSONValue JSON::parse_number() {
    size_t start = index;
    if (peek() == '-') get();
    if (peek() == '0') {
        get();
    } else if (std::isdigit(peek())) {
        while (std::isdigit(peek())) get();
    } else {
        throw JSONError("Invalid JSON number");
    }

    if (peek() == '.') {
        get();
        if (!std::isdigit(peek())) throw JSONError("Invalid JSON number");
        while (std::isdigit(peek())) get();
    }

    double number = std::stod(text.substr(start, index - start));
    return JSONValue(number); // Ensure the number is stored as double
}


JSONValue JSON::parse_string() {
    get(); // skip '"'
    std::string s;
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\\') {
            get();
            char escaped = get();
            switch (escaped) {
                case '"': s += '"'; break;
                case '\\': s += '\\'; break;
                case '/': s += '/'; break;
                case 'b': s += '\b'; break;
                case 'f': s += '\f'; break;
                case 'n': s += '\n'; break;
                case 'r': s += '\r'; break;
                case 't': s += '\t'; break;
                default: throw JSONError("Invalid escape character");
            }
        } else {
            s += get();
        }
    }
    if (get() != '"') throw JSONError("Unterminated string");
    return JSONValue(s);
}

JSONValue JSON::parse_array() {
    get(); // skip '['
    JSONArray array;
    skip_whitespace();
    if (peek() == ']') {
        get();
        return JSONValue(array);
    }
    while (true) {
        JSONValue val = parse_value();
        array.push_back(val);
        skip_whitespace();
        if (peek() == ',') {
            get();
        } else if (peek() == ']') {
            get();
            break;
        } else {
            throw JSONError("Expected ',' or ']'");
        }
    }
    return JSONValue(array);
}

JSONValue JSON::parse_object() {
    get(); // skip '{'
    JSONObject object;
    skip_whitespace();
    if (peek() == '}') {
        get();
        return JSONValue(object);
    }
    while (true) {
        skip_whitespace();
        if (peek() != '"') throw JSONError("Expected string key");
        JSONValue key = parse_string();
        skip_whitespace();
        if (get() != ':') throw JSONError("Expected ':'");
        skip_whitespace();
        JSONValue val = parse_value();
        object[key.as_string()] = val;
        skip_whitespace();
        if (peek() == ',') {
            get();
        } else if (peek() == '}') {
            get();
            break;
        } else {
            throw JSONError("Expected ',' or '}'");
        }
    }
    return JSONValue(object);
}

JSONValue JSON::parse(const std::string& text) {
    JSON parser(text);
    return parser.parse_value();
}

