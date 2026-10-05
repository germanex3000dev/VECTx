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

        if (number == 5) {
            conout("five");
        } else {
            conout("not five");
        }

        var any: value = 5;
        conout(value is(type: int));
        conout(value is(type: string));

        var any: label = greeting;
        if (label is(type: string)) {
            conout("greeting is a string");
        } else if (label is(type: int)) {
            conout("greeting is an int");
        } else {
            conout("greeting is something else");
        }

        var any: computed = value * 3;
        conout(computed is(type: int));

        var any: ratio = computed / 2;
        conout(ratio);
        conout(ratio is(type: float));

        if (ratio > 7) {
            var string: verdict = "greater than seven";
            conout(verdict);
        }

        var int: counter = 0;
        counter = counter + 5;
        conout(counter);

        if (counter > 0 && counter < 10) {
            conout("between zero and ten");
        }

        var any: mixed = 5;
        mixed = "now a string";
        conout(mixed is(type: string));
        conout(mixed is(type: int));

        if (mixed is(type: int) || counter == 5) {
            conout("int, or counter is five");
        }

        def func add(int: a, int: b) -> int: result {
            result = a + b;
        }

        def func describe(int: n) -> string: verdict {
            if (n > 7) {
                verdict = "big";
            } else {
                verdict = "small";
            }
        }

        conout(add(2, 3));
        conout(describe(add(4, 5)));

        conout("Type a number and a word, on two lines:");
        var int: typed = conin(type: int);
        var string: text = conin();
        conout(typed);
        conout(text);
        conout(typed * 2);
    )";

    Lexer lexer(source);

    auto tokens = lexer.tokenize();

    Parser parser(tokens);

    Program program = parser.parse();

    Interpreter interpreter;

    interpreter.execute(program);

    return 0;
}
