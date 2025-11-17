// TODO: add more function, varoiable, class, struct, module, Qt library, C++ standard library as you need.

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_COLOUR_NONE
#include "catch.hpp"

#include <string>
#include <sstream>


#include "interpreter_semantic_error.hpp"
#include "interpreter.hpp"
#include "expression.hpp"
#include "test_config.hpp"

// This is example unit test case with Catch 2
TEST_CASE("evaluating add", "[interpreter]") {
    std::string program = "(1 2 +)";
    std::istringstream iss(program);

    Interpreter interpreter;

    REQUIRE(interpreter.parse(iss));
    auto result = interpreter.eval();
    REQUIRE(result == Expression(3.));
}

// TODO: add more unit test cases to fully cover your code.
