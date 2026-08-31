#include "Lexer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"

int main() {

    const std::string source = R"(
        conout(42);
        conout(3.14159);
        conout(true);
        conout(false);
        conout("Hello VECTx");
        conout("Pineapples");
    )";

    Lexer lexer(source);

    auto tokens = lexer.tokenize();

    Parser parser(tokens);

    Program program = parser.parse();

    Interpreter interpreter;

    interpreter.execute(program);

    return 0;
}
