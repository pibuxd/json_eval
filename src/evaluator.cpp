#include "evaluator.h"
#include <sstream>
#include <cctype>
#include <algorithm>
#include <limits>
#include <future> // For std::async and std::future
#include <cmath>  // For abs, round, pow
#include <iostream> // For std::cout and std::endl

JSONValue Evaluator::evaluate(const std::string& expr) {
    return evaluate_expression(expr);
}

std::vector<std::string> Evaluator::parse_arguments(const std::string& args_str) {
    std::vector<std::string> args;
    size_t start = 0;
    int depth = 0;
    for (size_t i = 0; i < args_str.length(); ++i) {
        if (args_str[i] == '(') {
            depth++;
        } else if (args_str[i] == ')') {
            depth--;
        } else if (args_str[i] == ',' && depth == 0) {
            args.push_back(args_str.substr(start, i - start));
            start = i + 1;
        }
    }
    // Add the final argument
    args.push_back(args_str.substr(start));
    return args;
}


JSONValue Evaluator::evaluate_expression(const std::string& expr) {
    // Trim whitespace from both ends
    std::string trimmed_expr = expr;
    trimmed_expr.erase(0, trimmed_expr.find_first_not_of(" \t\n\r"));
    trimmed_expr.erase(trimmed_expr.find_last_not_of(" \t\n\r") + 1);

    // Check for functions (e.g., "max(...)", "min(...)", etc.)
    size_t func_pos = trimmed_expr.find('(');
    if (func_pos != std::string::npos && func_pos > 0) {
        // Look for matching closing bracket for the function call
        size_t end_pos = find_matching_bracket(trimmed_expr, func_pos);
        if (end_pos != std::string::npos) {
            std::string func_name = trimmed_expr.substr(0, func_pos);
            std::string args_str = trimmed_expr.substr(func_pos + 1, end_pos - func_pos - 1);

            // Parse arguments by splitting on commas, respecting nested expressions
            std::vector<std::string> args = parse_arguments(args_str);

            // Evaluate the function call
            return evaluate_function(func_name, args);
        }
    }

    // Check for specific binary operators (in descending order of precedence)
    if (expr.find("**") != std::string::npos) {
        size_t pos = expr.find("**");
        std::string left_expr = expr.substr(0, pos);
        std::string right_expr = expr.substr(pos + 2);
        JSONValue left = evaluate_expression(left_expr);
        JSONValue right = evaluate_expression(right_expr);

        if (!left.is_number() || !right.is_number()) throw EvalError("** requires numeric operands");
        return JSONValue(std::pow(left.as_number(), right.as_number()));
    }

    if (expr.find("&&") != std::string::npos) {
        size_t pos = expr.find("&&");
        std::string left_expr = expr.substr(0, pos);
        std::string right_expr = expr.substr(pos + 2);
        JSONValue left = evaluate_expression(left_expr);
        JSONValue right = evaluate_expression(right_expr);

        return JSONValue(left.as_number() && right.as_number());
    }

    if (expr.find("||") != std::string::npos) {
        size_t pos = expr.find("||");
        std::string left_expr = expr.substr(0, pos);
        std::string right_expr = expr.substr(pos + 2);
        JSONValue left = evaluate_expression(left_expr);
        JSONValue right = evaluate_expression(right_expr);

        return JSONValue(left.as_number() || right.as_number());
    }

    // Handle modulus
    size_t pos = expr.find('%');
    if (pos != std::string::npos) {
        std::string left_expr = expr.substr(0, pos);
        std::string right_expr = expr.substr(pos + 1);
        JSONValue left = evaluate_expression(left_expr);
        JSONValue right = evaluate_expression(right_expr);

        if (!left.is_number() || !right.is_number()) throw EvalError("% requires numeric operands");
        return JSONValue(static_cast<double>(static_cast<int>(left.as_number()) % static_cast<int>(right.as_number())));
    }

    // Check for basic arithmetic operators
    pos = expr.find_first_of("+-*/");
    if (pos != std::string::npos) {
        char op = expr[pos];
        std::string left_expr = expr.substr(0, pos);
        std::string right_expr = expr.substr(pos + 1);

        JSONValue left = evaluate_expression(left_expr);
        JSONValue right = evaluate_expression(right_expr);

        if (!left.is_number() || !right.is_number()) {
            throw EvalError("Arithmetic operations require numeric operands");
        }

        double l = left.as_number();
        double r = right.as_number();
        switch (op) {
            case '+': return JSONValue(l + r);
            case '-': return JSONValue(l - r);
            case '*': return JSONValue(l * r);
            case '/':
                if (r == 0) throw EvalError("Division by zero");
                return JSONValue(l / r);
        }
    }

    // Fallback to JSON path evaluation if no operators or functions are found
    return evaluate_json_path(trimmed_expr);
}


JSONValue Evaluator::evaluate_function(const std::string& func_name, const std::vector<std::string>& args) {
    if (func_name == "min") {
        double min_value = std::numeric_limits<double>::infinity();

        // Launch async tasks to evaluate each argument concurrently
        std::vector<std::future<JSONValue>> futures;
        for (const auto& arg : args) {
            futures.push_back(std::async(&Evaluator::evaluate_expression, this, arg));
        }

        // Collect results and find the minimum value
        for (auto& future : futures) {
            JSONValue val = future.get();
            if (val.is_array()) {
                for (const auto& item : val.as_array()) {
                    if (!item.is_number()) throw EvalError("min requires numeric values");
                    min_value = std::min(min_value, item.as_number());
                }
            } else if (val.is_number()) {
                min_value = std::min(min_value, val.as_number());
            } else {
                throw EvalError("min requires numeric values");
            }
        }
        if (min_value == std::numeric_limits<double>::infinity()) throw EvalError("min requires at least one numeric value");
        return JSONValue(min_value);
    } else if (func_name == "max") {
        double max_value = -std::numeric_limits<double>::infinity();

        // Launch async tasks to evaluate each argument concurrently
        std::vector<std::future<JSONValue>> futures;
        for (const auto& arg : args) {
            futures.push_back(std::async(&Evaluator::evaluate_expression, this, arg));
        }

        // Collect results and find the maximum value
        for (auto& future : futures) {
            JSONValue val = future.get();
            if (val.is_array()) {
                for (const auto& item : val.as_array()) {
                    if (!item.is_number()) throw EvalError("max requires numeric values");
                    max_value = std::max(max_value, item.as_number());
                }
            } else if (val.is_number()) {
                max_value = std::max(max_value, val.as_number());
            } else {
                throw EvalError("max requires numeric values");
            }
        }
        if (max_value == -std::numeric_limits<double>::infinity()) throw EvalError("max requires at least one numeric value");
        return JSONValue(max_value);
    } else if (func_name == "size") {
        if (args.size() != 1) throw EvalError("size requires exactly one argument");
        JSONValue val = evaluate_expression(args[0]);
        if (val.is_string()) {
            return JSONValue(static_cast<double>(val.as_string().size()));
        } else if (val.is_array()) {
            return JSONValue(static_cast<double>(val.as_array().size()));
        } else if (val.is_object()) {
            return JSONValue(static_cast<double>(val.as_object().size()));
        } else {
            throw EvalError("size requires an object, array, or string");
        }
    } else if (func_name == "sum") {
        double sum = 0.0;
        
        // Launch async tasks to evaluate each argument concurrently
        std::vector<std::future<JSONValue>> futures;
        for (const auto& arg : args) {
            futures.push_back(std::async(&Evaluator::evaluate_expression, this, arg));
        }

        // Collect results and calculate the sum
        for (auto& future : futures) {
            JSONValue val = future.get();
            if (val.is_array()) {
                for (const auto& item : val.as_array()) {
                    if (!item.is_number()) throw EvalError("sum requires numeric values");
                    sum += item.as_number();
                }
            } else if (val.is_number()) {
                sum += val.as_number();
            } else {
                throw EvalError("sum requires numeric values");
            }
        }
        return JSONValue(sum);
    } else if (func_name == "avg") {
        double sum = 0.0;
        int count = 0;

        std::vector<std::future<JSONValue>> futures;
        for (const auto& arg : args) {
            futures.push_back(std::async(&Evaluator::evaluate_expression, this, arg));
        }

        for (auto& future : futures) {
            JSONValue val = future.get();
            if (val.is_array()) {
                for (const auto& item : val.as_array()) {
                    if (!item.is_number()) throw EvalError("avg requires numeric values");
                    sum += item.as_number();
                    count++;
                }
            } else if (val.is_number()) {
                sum += val.as_number();
                count++;
            } else {
                throw EvalError("avg requires numeric values");
            }
        }

        if (count == 0) throw EvalError("avg requires at least one numeric value");
        return JSONValue(sum / count);
    } else if (func_name == "count") {
        if (args.size() != 1) throw EvalError("count requires exactly one argument");
        JSONValue val = evaluate_expression(args[0]);
        if (val.is_array()) {
            return JSONValue(static_cast<double>(val.as_array().size()));
        } else if (val.is_string()) {
            return JSONValue(static_cast<double>(val.as_string().size()));
        } else {
            throw EvalError("count requires an array or string");
        }
    } else if (func_name == "abs") {
        if (args.size() != 1) throw EvalError("abs requires exactly one argument");
        JSONValue val = evaluate_expression(args[0]);
        if (!val.is_number()) throw EvalError("abs requires a numeric value");
        return JSONValue(std::abs(val.as_number()));
    } else if (func_name == "round") {
        if (args.size() != 1) throw EvalError("round requires exactly one argument");
        JSONValue val = evaluate_expression(args[0]);
        if (!val.is_number()) throw EvalError("round requires a numeric value");
        return JSONValue(std::round(val.as_number()));
    } else {
        throw EvalError("Unknown function: " + func_name);
    }
}

JSONValue Evaluator::evaluate_json_path(const std::string& path) {
    JSONValue current = root;
    size_t pos = 0;

    // Read the initial key segment (e.g., "a" in "a.b[2]")
    while (pos < path.size() && (std::isalnum(path[pos]) || path[pos] == '_')) ++pos;
    std::string key = path.substr(0, pos);
    current = get_value(current, key);

    while (pos < path.size()) {
        if (path[pos] == '.') {
            ++pos;
            size_t start = pos;
            while (pos < path.size() && (std::isalnum(path[pos]) || path[pos] == '_')) ++pos;
            key = path.substr(start, pos - start);

            if (!current.is_object()) {
                throw EvalError("Invalid key access on non-object type: " + key);
            }
            current = get_value(current, key);
        } else if (path[pos] == '[') {
            ++pos;
            size_t start = pos;
            while (pos < path.size() && path[pos] != ']') ++pos;
            if (pos >= path.size() || path[pos] != ']') {
                throw EvalError("Expected ']' for array index access.");
            }

            std::string index_str = path.substr(start, pos - start);
            ++pos; // Move past ']'

            if (!index_str.empty() && std::all_of(index_str.begin(), index_str.end(), ::isdigit)) {
                size_t index = std::stoul(index_str);
                if (!current.is_array()) {
                    throw EvalError("Invalid array index access on non-array type.");
                }
                const JSONArray& arr = current.as_array();
                if (index >= arr.size()) {
                    throw EvalError("Array index out of bounds: " + index_str);
                }
                current = arr[index];
            } else {
                throw EvalError("Invalid array index: " + index_str + " (must be an integer).");
            }
        } else {
            throw EvalError("Unexpected syntax or character in path: " + path.substr(pos));
        }
    }

    return current;
}


size_t Evaluator::find_matching_bracket(const std::string& s, size_t pos) {
    int depth = 1;
    for (size_t i = pos + 1; i < s.size(); ++i) {
        if (s[i] == '(') depth++;
        else if (s[i] == ')') depth--;
        if (depth == 0) return i;
    }
    throw EvalError("Mismatched parentheses");
}

JSONValue Evaluator::get_value(const JSONValue& current, const std::string& key) {
    if (current.is_object()) {
        const JSONObject& obj = current.as_object();
        auto it = obj.find(key);
        if (it != obj.end()) return it->second;
        else throw EvalError("Key not found: " + key);
    } else if (current.is_array()) {
        try {
            size_t index = std::stoul(key);
            const JSONArray& arr = current.as_array();
            if (index >= arr.size()) throw EvalError("Array index out of bounds: " + key);
            return arr[index]; // Return directly without altering
        } catch (const std::invalid_argument&) {
            throw EvalError("Invalid array index: " + key);
        } catch (const std::out_of_range&) {
            throw EvalError("Array index out of range: " + key);
        }
    } else {
        throw EvalError("Current value is not an object or array");
    }
}

