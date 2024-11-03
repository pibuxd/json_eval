#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "json.h" // Include your JSON class
#include "evaluator.h" // Include your Evaluator class

// Parsing the updated JSON structure
JSONValue json = JSON::parse(R"({
    "a": {
        "b": [0, 0, 1, 2, 3, [0, 10, 20], { "c": "test" }]
    }
})");

Evaluator evaluator(json);

TEST_CASE("Basic JSON Path Access") {
    REQUIRE(evaluator.evaluate("a.b[0]").as_number() == 0); // Accessing the first element
    REQUIRE(evaluator.evaluate("a.b[1]").as_number() == 0); // Accessing the second element
    REQUIRE(evaluator.evaluate("a.b[2]").as_number() == 1); // Accessing the third element
    REQUIRE(evaluator.evaluate("a.b[3]").as_number() == 2); // Accessing the fourth element
    REQUIRE(evaluator.evaluate("a.b[4]").as_number() == 3); // Accessing the fifth element
}

TEST_CASE("Access Nested Array and Object") {
    REQUIRE(evaluator.evaluate("a.b[5][1]").as_number() == 10); // Accessing first element of nested array
    REQUIRE(evaluator.evaluate("a.b[5][2]").as_number() == 20); // Accessing second element of nested array
    REQUIRE(evaluator.evaluate("a.b[6].c").as_string() == "test"); // Accessing value of key "c" in nested object
}

TEST_CASE("Arithmetic Operations") {
    REQUIRE(evaluator.evaluate("a.b[2] + a.b[3]").as_number() == 3); // 1 + 2
    REQUIRE(evaluator.evaluate("a.b[4] * a.b[3]").as_number() == 6); // 3 * 2
    REQUIRE(evaluator.evaluate("a.b[5][2] - a.b[2]").as_number() == 19); // 20 - 1
    REQUIRE(evaluator.evaluate("a.b[5][1] / a.b[2]").as_number() == 10); // 10 / 1
}

TEST_CASE("Complex Expressions") {
    REQUIRE(evaluator.evaluate("a.b[5][1] % a.b[4]").as_number() == 1); // 10 % 3
    REQUIRE(evaluator.evaluate("a.b[2] * a.b[3] + a.b[4]").as_number() == 5); // 1 * (2 + 3)
}

TEST_CASE("Logical Operations") {
    REQUIRE(evaluator.evaluate("a.b[0] && a.b[2]").as_bool() == false); // 0 && 1 -> false
    REQUIRE(evaluator.evaluate("a.b[2] || a.b[1]").as_bool() == true); // 1 || 0 -> true
}
