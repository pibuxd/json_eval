
# JSON Expression Evaluator

## Project Overview
The **JSON Expression Evaluator** is a C++ project designed to parse and evaluate expressions over JSON data. This project supports arithmetic, logical operations, and JSON path traversal, enabling users to perform complex evaluations on JSON objects.

## Features
- **JSON Parsing**: Parses JSON strings into structured `JSONValue` objects.
- **Expression Evaluation**: Supports arithmetic (`+`, `-`, `*`, `/`, `%`, `**`), logical (`&&`, `||`, `!`) operations, and functions (`min`, `max`, `sum`, `avg`, `size`, `abs`, `round`).
- **JSON Path Traversal**: Accesses values within nested JSON structures using paths like `a.b[0]`.
- **Function Calls**: Evaluates expressions with function calls such as `min(1, 2, 3)` or `size(array)`.

## Directory Structure
```
json_eval/
│
├── src/
│   ├── main.cpp          # Entry point of the application
│   ├── evaluator.cpp     # Core evaluator implementation
│   ├── evaluator.h       # Header file for the evaluator
│   ├── json.cpp          # JSON parsing and handling
│   ├── json.h            # Header file for JSON class
│   └── test.cpp          # Unit tests using Catch2
│
├── include/              # Include headers (if separated)
│
├── CMakeLists.txt        # CMake configuration file
└── README.md             # Project description
```

## Dependencies
- **C++17** or newer
- **Catch2** for testing (optional but recommended)

## Installation and Compilation
1. **Clone the repository**:
    ```bash
    git clone https://github.com/yourusername/json_eval.git
    cd json_eval
    ```

2. **Compile the project**:
    ```bash
    make
    ```

3. **Run the program**:
    ```bash
    ./json_eval path/to/json/file.json "expression"
    ```

## Usage Example
Given a JSON file `test.json`:
```json
{
    "a": {
        "b": [0, 0, 1, 2, 3, [10, 20], { "c": "test" }]
    }
}
```

### Running Evaluations:
```bash
./json_eval test.json "a.b[0] + a.b[1]"    # Output: 0
./json_eval test.json "a.b[2] * a.b[3]"    # Output: 2
./json_eval test.json "a.b[6].c"           # Output: "test"
```

## Testing
Unit tests are included in `test.cpp` and can be run using Catch2:

1. **Compile tests**:
    ```bash
    g++ -std=c++17 src/test.cpp src/evaluator.cpp src/json.cpp -o test_executable -I/path/to/catch
    ```

2. **Run tests**:
    ```bash
    ./test_executable
    ```

## Project Details
### Core Components:
- **`JSON` Class**: Handles parsing of JSON strings and conversion to `JSONValue`.
- **`Evaluator` Class**: Parses and evaluates expressions, supports arithmetic, logical operations, function calls, and JSON path evaluation.

### Functionality:
- **Arithmetic Operations**: Supports expressions such as `5 + 10`, `3 * (2 + 4)`.
- **Logical Operations**: Evaluate `true && false`, `a.b[0] || 0`.
- **Built-in Functions**: 
    - `min(a, b, c)`, `max(a, b, c)`
    - `sum(array)`, `avg(array)`
    - `size(object_or_array)`, `abs(number)`, `round(number)`
