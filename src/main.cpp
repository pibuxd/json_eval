#include <iostream>
#include <fstream>
#include <sstream>
#include "json.h"
#include "evaluator.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./json_eval <json_file> \"<expression>\"" << std::endl;
        return 1;
    }

    std::ifstream json_file(argv[1]);
    if (!json_file.is_open()) {
        std::cerr << "Error: Could not open JSON file." << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << json_file.rdbuf();
    std::string json_content = buffer.str();

    try {
        JSONValue json = JSON::parse(json_content);
        Evaluator evaluator(json);
        JSONValue result = evaluator.evaluate(argv[2]);
        std::cout << result.to_string() << std::endl;
    } catch (const JSONError& e) {
        std::cerr << "JSON Error: " << e.what() << std::endl;
        return 1;
    } catch (const EvalError& e) {
        std::cerr << "Evaluation Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

