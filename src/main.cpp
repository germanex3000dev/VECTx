#include "Lexer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"

int main() {

    const std::string source = R"(
        var int: number = 5;
        conout(number);
        conout(number * 3);

        var int: doubled = number * 2;
        conout(doubled);

        var float: pi = 3.14;
        conout(pi);

        var string: greeting = "Pineapples";
        conout(greeting);

        var bool: truthy = true;
        conout(truthy);

        var any: value = 5;
        conout(value);
        conout(value is(type: int));
        conout(value is(type: string));

        var any: computed = value * 3;
        conout(computed is(type: int));

        var any: ratio = computed / 2;
        conout(ratio);
        conout(ratio is(type: float));
    )";

    Lexer lexer(source);

    auto tokens = lexer.tokenize();

    Parser parser(tokens);

    Program program = parser.parse();

    Interpreter interpreter;

    interpreter.execute(program);

    return 0;
}
