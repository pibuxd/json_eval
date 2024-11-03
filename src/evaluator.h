#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "json.h"
#include <string>
#include <exception>
#include <map>

class EvalError : public std::exception {
    std::string message;
public:
    EvalError(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class Evaluator {
    JSONValue root;

    JSONValue evaluate_expression(const std::string& expr);
    JSONValue evaluate_json_path(const std::string& path);
    JSONValue evaluate_function(const std::string& func_name, const std::vector<std::string>& args);
    std::vector<std::string> parse_arguments(const std::string& args_str);

    // Helper functions
    size_t find_matching_bracket(const std::string& s, size_t pos);
    JSONValue get_value(const JSONValue& current, const std::string& key);

public:
    Evaluator(const JSONValue& json_root) : root(json_root) {}
    JSONValue evaluate(const std::string& expr);
};

#endif

