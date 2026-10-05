// Tests for the parts of VECTx that are hard to observe from a script:
// scoping, scope cleanup when execution aborts, and the operator semantics.
// Run with `ctest --test-dir build` or by executing this binary directly.

#include "Environment.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Value.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

int failures = 0;
int checks = 0;

// Runs a program on an existing interpreter, capturing whatever it prints.
std::string run(Interpreter& interpreter, const std::string& source) {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    Program program = parser.parse();

    std::ostringstream buffer;
    std::streambuf* previous = std::cout.rdbuf(buffer.rdbuf());

    try {
        interpreter.execute(program);
    } catch (...) {
        std::cout.rdbuf(previous);
        throw;
    }

    std::cout.rdbuf(previous);
    return buffer.str();
}

std::string run(const std::string& source) {
    Interpreter interpreter;
    return run(interpreter, source);
}

// Runs a program with `input` standing in for stdin, and returns what it
// printed. conin() reads a line at a time from it.
std::string runWithInput(
    const std::string& source,
    const std::string& input
) {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    Program program = parser.parse();

    std::istringstream inputStream(input);
    std::streambuf* previousInput = std::cin.rdbuf(inputStream.rdbuf());

    std::ostringstream buffer;
    std::streambuf* previousOutput = std::cout.rdbuf(buffer.rdbuf());

    try {
        Interpreter interpreter;
        interpreter.execute(program);
    } catch (...) {
        std::cout.rdbuf(previousOutput);
        std::cin.rdbuf(previousInput);
        throw;
    }

    std::cout.rdbuf(previousOutput);
    std::cin.rdbuf(previousInput);
    return buffer.str();
}

void pass(const std::string& label) {
    ++checks;
    std::cout << "  ok    " << label << "\n";
}

void fail(const std::string& label, const std::string& detail) {
    ++checks;
    ++failures;
    std::cout << "  FAIL  " << label << "\n" << detail;
}

std::string show(const std::string& text) {
    return "          got: \"" + text + "\"\n";
}

// Asserts a program prints exactly `expected`, where "\n" separates lines.
void expectOutput(
    const std::string& label,
    const std::string& source,
    const std::string& expected
) {
    std::string actual;

    try {
        actual = run(source);
    } catch (const std::exception& e) {
        fail(label, std::string("          unexpected error: ") + e.what() + "\n");
        return;
    }

    if (actual == expected) {
        pass(label);
    } else {
        fail(label, show(actual));
    }
}

// Asserts a program fails with a message containing `fragment`.
void expectError(
    const std::string& label,
    const std::string& source,
    const std::string& fragment
) {
    std::string message;

    try {
        run(source);
    } catch (const std::exception& e) {
        message = e.what();
    }

    if (message.empty()) {
        fail(label, "          expected an error, none was raised\n");
    } else if (message.find(fragment) != std::string::npos) {
        pass(label);
    } else {
        fail(
            label,
            std::string("          wanted error containing: ") + fragment +
            "\n" + show(message)
        );
    }
}

void testIfStatements() {
    std::cout << "if statements\n";

    expectOutput(
        "if takes the then branch",
        "var int: x = 3; if (x == 3) { conout(\"three\"); }",
        "three\n"
    );

    expectOutput(
        "if skips a false branch",
        "var int: x = 3; if (x == 4) { conout(\"no\"); } conout(\"after\");",
        "after\n"
    );

    expectOutput(
        "else runs when the condition is false",
        "var int: x = 4; if (x == 3) { conout(\"a\"); } else { conout(\"b\"); }",
        "b\n"
    );

    expectOutput(
        "else if picks the matching branch",
        "var int: x = 7;"
        " if (x == 3) { conout(\"a\"); }"
        " else if (x == 7) { conout(\"b\"); }"
        " else { conout(\"c\"); }",
        "b\n"
    );

    expectOutput(
        "the final else catches the rest",
        "var int: x = 9;"
        " if (x == 3) { conout(\"a\"); }"
        " else if (x == 7) { conout(\"b\"); }"
        " else { conout(\"c\"); }",
        "c\n"
    );

    expectOutput(
        "a body may be a single statement",
        "if (true) conout(1); conout(2);",
        "1\n2\n"
    );

    expectOutput(
        "branches may nest",
        "var int: x = 5;"
        " if (x > 0) { if (x > 3) { conout(\"big\"); } else { conout(\"small\"); } }"
        " else { conout(\"neg\"); }",
        "big\n"
    );

    expectOutput(
        "a false branch does not run its body",
        "if (false) { conout(1 / 0); } conout(\"survived\");",
        "survived\n"
    );
}

void testTypeChecksInConditions() {
    std::cout << "is(type:) in conditions\n";

    expectOutput(
        "is(type: int) is true for an int",
        "var any: v = 5; if (v is(type: int)) { conout(\"int\"); } else { conout(\"no\"); }",
        "int\n"
    );

    expectOutput(
        "is(type: string) is false for an int",
        "var any: v = 5; if (v is(type: string)) { conout(\"s\"); } else { conout(\"no\"); }",
        "no\n"
    );

    expectOutput(
        "is(type: any) accepts a value",
        "var any: v = 5; if (v is(type: any)) { conout(\"any\"); }",
        "any\n"
    );

    expectOutput(
        "is(type: any) rejects a value with no data",
        "var any: v = conout(1); if (v is(type: any)) { conout(\"any\"); } else { conout(\"none\"); }",
        "1\nnone\n"
    );

    expectOutput(
        "a type check combines with ==",
        "var any: v = 5; if (v is(type: string) == false) { conout(\"ok\"); }",
        "ok\n"
    );
}

void testConditionsMustBeBoolean() {
    std::cout << "conditions must be boolean\n";

    expectError("if (1) is rejected", "if (1) { }", "must be a bool, got int");
    expectError(
        "if (x) is rejected",
        "var any: x = 5; if (x) { }",
        "must be a bool, got int"
    );
    expectError(
        "if (\"s\") is rejected",
        "if (\"s\") { }",
        "must be a bool, got string"
    );
    expectError(
        "if (1 + 2) is rejected",
        "if (1 + 2) { }",
        "must be a bool, got int"
    );

    expectOutput(
        "a comparison is a valid condition",
        "if (1 < 2) { conout(\"ok\"); }",
        "ok\n"
    );
}

void testScoping() {
    std::cout << "block scoping\n";

    expectError(
        "a variable declared in a block does not leak",
        "if (true) { var int: inner = 5; conout(inner); } conout(inner);",
        "Undefined variable: inner"
    );

    expectOutput(
        "an outer variable is visible inside a block",
        "var int: g = 1; if (true) { conout(g); }",
        "1\n"
    );

    expectOutput(
        "an inner declaration shadows the outer one",
        "var int: x = 1; if (true) { var int: x = 99; conout(x); } conout(x);",
        "99\n1\n"
    );

    expectOutput(
        "sibling blocks do not share variables",
        "if (true) { var int: a = 1; conout(a); }"
        " if (true) { var int: a = 2; conout(a); }",
        "1\n2\n"
    );

    expectOutput(
        "shadowing unwinds through nested blocks",
        "var int: v = 0;"
        " if (true) { var int: v = 1;"
        "   if (true) { var int: v = 2; conout(v); }"
        "   conout(v);"
        " }"
        " conout(v);",
        "2\n1\n0\n"
    );

    expectOutput(
        "a skipped branch does not declare anything",
        "if (false) { var int: never = 1; } conout(\"done\");",
        "done\n"
    );

    expectOutput(
        "a block may declare and use a local",
        "var int: n = 3;"
        " if (n == 3) { var int: doubled = n * 2; conout(doubled); }",
        "6\n"
    );
}

void testSyntaxErrors() {
    std::cout << "syntax errors\n";

    expectError("missing ( after if", "if x == 3) { }", "Expected '(' after 'if'");
    expectError("missing ) after condition", "if (x == 3 { }", "Expected ')' after an if condition");
    expectError("unclosed block", "if (true) { conout(1);", "Expected '}' to close a block");
    expectError("empty condition", "if () { }", "Expected expression");
}

// An error thrown inside nested blocks must not leave scopes on the stack, or
// every later lookup would see the abandoned block's variables.
void testScopeReleasedAfterError() {
    ++checks;

    Interpreter interpreter;
    std::string label = "scopes are released when a block throws";

    try {
        run(interpreter, "if (true) { if (true) { conout(missing); } }");
    } catch (const std::exception&) {
        // expected
    }

    try {
        std::string output = run(interpreter, "var int: after = 5; conout(after);");

        if (output == "5\n") {
            pass(label);
        } else {
            fail(label, show(output));
        }
    } catch (const std::exception& e) {
        fail(
            label,
            std::string("          interpreter was left in a bad state: ") + e.what() + "\n"
        );
    }
}

// assign() updating a binding in an enclosing scope is what a future
// `def f(...) -> ret_glob { return v }` will rely on.
void testAssignReachesEnclosingScope() {
    std::cout << "environment\n";

    ++checks;
    std::string label = "assign() updates a binding in an outer scope";

    Environment environment;
    environment.define("ret_glob", Value(), VariableType::Any);
    environment.pushScope();
    environment.define("local", Value(7), VariableType::Integer);
    environment.assign("ret_glob", Value(42));
    environment.popScope();

    if (environment.get("ret_glob").asInt() == 42) {
        pass(label);
    } else {
        fail(label, "          the outer binding was not updated\n");
    }

    ++checks;
    label = "a popped scope discards its own bindings";

    bool gone = false;
    try {
        environment.get("local");
    } catch (const std::exception&) {
        gone = true;
    }

    if (gone) {
        pass(label);
    } else {
        fail(label, "          'local' survived the scope it was declared in\n");
    }

    ++checks;
    label = "assign() to an undefined name is an error";

    bool threw = false;
    try {
        environment.assign("ghost", Value(1));
    } catch (const std::exception&) {
        threw = true;
    }

    if (threw) {
        pass(label);
    } else {
        fail(label, "          no error was raised\n");
    }
}

void testIntegerArithmetic() {
    std::cout << "arithmetic\n";

    expectOutput(
        "int op int stays an int",
        "var int: a = 5 + 3; conout(a); conout(a is(type: int));",
        "8\ntrue\n"
    );

    expectOutput(
        "division widens to float",
        "var any: a = 7 / 2; conout(a); conout(a is(type: float));",
        "3.5\ntrue\n"
    );

    expectError(
        "overflow is reported",
        "var int: x = 2000000000 + 2000000000;",
        "Integer overflow"
    );
}

void expectInputOutput(
    const std::string& label,
    const std::string& source,
    const std::string& input,
    const std::string& expected
) {
    std::string actual;

    try {
        actual = runWithInput(source, input);
    } catch (const std::exception& e) {
        fail(label, std::string("          unexpected error: ") + e.what() + "\n");
        return;
    }

    if (actual == expected) {
        pass(label);
    } else {
        fail(label, show(actual));
    }
}

void expectInputError(
    const std::string& label,
    const std::string& source,
    const std::string& input,
    const std::string& fragment
) {
    std::string message;

    try {
        runWithInput(source, input);
    } catch (const std::exception& e) {
        message = e.what();
    }

    if (message.empty()) {
        fail(label, "          expected an error, none was raised\n");
    } else if (message.find(fragment) != std::string::npos) {
        pass(label);
    } else {
        fail(
            label,
            std::string("          wanted error containing: ") + fragment +
            "\n" + show(message)
        );
    }
}

void testConin() {
    std::cout << "conin()\n";

    expectInputOutput(
        "conin() reads a line as a string",
        "conout(conin());",
        "hello\n",
        "hello\n"
    );

    expectInputOutput(
        "a line without a trailing newline is read",
        "var string: line = conin(); conout(line);",
        "no newline",
        "no newline\n"
    );

    expectInputOutput(
        "conin() reads one line per call",
        "conout(conin()); conout(conin());",
        "first\nsecond\n",
        "first\nsecond\n"
    );

    expectInputOutput(
        "the result is always a string",
        "var any: v = conin(); conout(v is(type: string)); conout(v is(type: int));",
        "42\n",
        "true\nfalse\n"
    );

    expectInputOutput(
        "a string result feeds an int declaration after a type check",
        "var string: text = conin();"
        " if (text is(type: string) && text != \"\") { conout(\"got something\"); }",
        "something\n",
        "got something\n"
    );

    expectInputOutput(
        "conin() works as a function argument",
        "def func echo(string: a) -> string: out { out = a; } conout(echo(conin()));",
        "through a function\n",
        "through a function\n"
    );

    expectInputOutput(
        "an empty line is an empty string, not a failure",
        "var string: line = conin(); conout(line); conout(\"|\");",
        "\n",
        "\n|\n"
    );

    expectInputError(
        "the end of input without a line is an error",
        "conout(conin());",
        "",
        "end of input"
    );

    expectInputError(
        "conin() takes no arguments",
        "conout(conin(1));",
        "x\n",
        "conin() expects no arguments"
    );

    expectInputError(
        "a wrongly typed conin() result is rejected",
        "var int: n = conin();",
        "5\n",
        "Type mismatch"
    );
}

void testConinTyped() {
    std::cout << "conin(type:)\\n";

    expectInputOutput(
        "conin(type: int) reads an int",
        "var int: num = conin(type: int); conout(num); conout(num is(type: int));",
        "42\n",
        "42\ntrue\n"
    );

    expectInputOutput(
        "a typed int does arithmetic without a check",
        "var int: a = conin(type: int); var int: b = conin(type: int); conout(a + b);",
        "20\n22\n",
        "42\n"
    );

    expectInputOutput(
        "conin(type: float) reads a float",
        "var float: f = conin(type: float); conout(f); conout(f is(type: float));",
        "3.5\n",
        "3.5\ntrue\n"
    );

    expectInputOutput(
        "a typed float may be a whole number",
        "var float: f = conin(type: float); conout(f is(type: float));",
        "4\n",
        "true\n"
    );

    expectInputError(
        "an int input is not widened to float",
        "var float: f = conin(type: int);",
        "4\n",
        "Type mismatch"
    );

    expectInputOutput(
        "conin(type: bool) reads true and false",
        "var bool: a = conin(type: bool); var bool: b = conin(type: bool); conout(a); conout(b);",
        "true\nfalse\n",
        "true\nfalse\n"
    );

    expectInputOutput(
        "conin(type: string) reads a string",
        "var string: s = conin(type: string); conout(s);",
        "text\n",
        "text\n"
    );

    expectInputOutput(
        "conin(type: any) reads a string with no conversion",
        "var any: v = conin(type: any); conout(v is(type: string));",
        "5\n",
        "true\n"
    );

    expectInputOutput(
        "surrounding spaces are tolerated by a typed read",
        "var int: n = conin(type: int); conout(n);",
        "  7  \n",
        "7\n"
    );

    expectInputOutput(
        "a blank line is skipped by a typed read",
        "var int: n = conin(type: int); conout(n);",
        "\n\n9\n",
        "9\n"
    );

    expectInputOutput(
        "a typed read works as a function argument",
        "def func double_it(int: a) -> int: out { out = a * 2; }"
        " conout(double_it(conin(type: int)));",
        "21\n",
        "42\n"
    );

    expectInputOutput(
        "a typed read drives a condition",
        "var int: age = conin(type: int);"
        " if (age >= 18) { conout(\"adult\"); } else { conout(\"minor\"); }",
        "21\n",
        "adult\n"
    );

    expectInputError(
        "text for a typed int is an error",
        "var int: n = conin(type: int);",
        "hello\n",
        "expected a number, got \"hello\""
    );

    expectInputError(
        "a partly numeric line is an error",
        "var int: n = conin(type: int);",
        "12abc\n",
        "expected a number, got \"12abc\""
    );

    expectInputError(
        "an int too large for int is an error",
        "var int: n = conin(type: int);",
        "99999999999\n",
        "expected a number"
    );

    expectInputError(
        "text for a typed float is an error",
        "var float: f = conin(type: float);",
        "nope\n",
        "expected a number, got \"nope\""
    );

    expectInputError(
        "anything but true or false is an error",
        "var bool: b = conin(type: bool);",
        "yes\n",
        "expected 'true' or 'false', got \"yes\""
    );

    expectInputError(
        "a type cannot be mixed with arguments",
        "conout(conin(type: int, 1));",
        "5\n",
        "conin(type: ...) takes no arguments"
    );

    expectInputError(
        "an argument alone is still rejected",
        "conout(conin(1));",
        "5\n",
        "conin() expects no arguments"
    );
}

void testFunctions() {
    std::cout << "functions\n";

    expectOutput(
        "a function returns its declared return variable",
        "def func add(int: a, int: b) -> int: result { result = a + b; }"
        " conout(add(2, 3));",
        "5\n"
    );

    expectOutput(
        "a function may set the return variable in a branch",
        "def func describe(int: n) -> string: verdict {"
        " if (n > 7) { verdict = \"big\"; } else { verdict = \"small\"; } }"
        " conout(describe(4 + 5));",
        "big\n"
    );

    expectOutput(
        "return with a value yields that value",
        "def func pick(int: a, int: b) -> int: out { return a * 10 + b; }"
        " conout(pick(1, 2));",
        "12\n"
    );

    expectOutput(
        "a bare return yields the return variable",
        "def func bump(int: a) -> int: out { out = a + 1; return; }"
        " conout(bump(1));",
        "2\n"
    );

    expectOutput(
        "falling off the end yields the return variable",
        "def func half(int: a) -> any: out { out = a / 2; }"
        " conout(half(7));",
        "3.5\n"
    );

    expectOutput(
        "an untouched return variable starts at its type's zero",
        "def func nothing() -> int: out { } conout(nothing());",
        "0\n"
    );

    expectOutput(
        "a function may be declared after its use",
        "conout(twice(4)); def func twice(int: a) -> int: out { out = a * 2; }",
        "8\n"
    );

    expectOutput(
        "a function may call another function",
        "def func inner(int: a) -> int: out { out = a + 1; }"
        " def func outer(int: a) -> int: out { out = inner(a) * 2; }"
        " conout(outer(3));",
        "8\n"
    );

    expectOutput(
        "a function with no parameters takes none",
        "def func five() -> int: out { out = 5; } conout(five());",
        "5\n"
    );

    expectOutput(
        "an any argument fits an any parameter",
        "def func show(any: v) -> bool: out { out = v is(type: string); }"
        " conout(show(\"text\"));",
        "true\n"
    );

    expectError(
        "a wrongly typed argument is rejected",
        "def func need_int(int: a) -> int: out { out = a; } conout(need_int(\"no\"));",
        "Type mismatch in argument 'a' of need_int"
    );

    expectError(
        "the wrong argument count is rejected",
        "def func add(int: a, int: b) -> int: out { out = a + b; } conout(add(1));",
        "add() expects 2 argument(s), got 1"
    );

    expectError(
        "a return value of the wrong type is rejected",
        "def func bad() -> int: out { return \"no\"; } conout(bad());",
        "Return type mismatch in bad"
    );

    expectError(
        "calling an unknown function is rejected",
        "conout(nope(1));",
        "Unknown function: nope"
    );

    expectError(
        "a duplicate function name is rejected",
        "def func f() -> int: out { } def func f() -> int: out { }",
        "Function already declared: f"
    );

    expectError(
        "a missing -> is a syntax error",
        "def func f(int: a) { }",
        "Expected '->'"
    );

    expectError(
        "a missing parameter type is a syntax error",
        "def func f(a) -> int: out { }",
        "Expected a type name"
    );
}

void testFunctionScope() {
    std::cout << "function scope\n";

    expectError(
        "a parameter does not leak out",
        "def func f(int: a) -> int: out { out = a; } conout(a);",
        "Undefined variable: a"
    );

    expectError(
        "the return variable does not leak out",
        "def func f(int: a) -> int: out { out = a; } conout(out);",
        "Undefined variable: out"
    );

    expectError(
        "a local does not leak out",
        "def func f(int: a) -> int: out { var int: tmp = a; out = tmp; } conout(tmp);",
        "Undefined variable: tmp"
    );

    expectOutput(
        "a function reads a global variable",
        "var int: g = 7; def func f() -> int: out { out = g; } conout(f());",
        "7\n"
    );

    expectOutput(
        "a call leaves no scope behind",
        "var int: x = 1;"
        " def func f(int: a) -> int: out { out = a + 1; }"
        " f(f(1));"
        " conout(x);",
        "1\n"
    );
}

void testLogicalOperators() {
    std::cout << "logical operators\n";

    expectOutput(
        "&& is true when both sides hold",
        "if (true && true) { conout(\"yes\"); } else { conout(\"no\"); }",
        "yes\n"
    );

    expectOutput(
        "&& is false when one side fails",
        "if (true && false) { conout(\"yes\"); } else { conout(\"no\"); }",
        "no\n"
    );

    expectOutput(
        "|| is true when one side holds",
        "if (false || true) { conout(\"yes\"); } else { conout(\"no\"); }",
        "yes\n"
    );

    expectOutput(
        "a comparison combines with &&",
        "var int: n = 5;"
        " if (n > 1 && n < 10) { conout(\"in range\"); } else { conout(\"out\"); }",
        "in range\n"
    );

    expectOutput(
        "a type check combines with ||",
        "var any: v = 5;"
        " if (v is(type: string) || v is(type: int)) { conout(\"ok\"); } else { conout(\"no\"); }",
        "ok\n"
    );

    expectOutput(
        "&& binds tighter than ||",
        "if (false && false || true) { conout(\"ok\"); } else { conout(\"no\"); }",
        "ok\n"
    );

    expectOutput(
        "&& short-circuits a false left side",
        "if (false && (1 / 0 == 0)) { conout(\"no\"); } conout(\"survived\");",
        "survived\n"
    );

    expectOutput(
        "|| short-circuits a true left side",
        "if (true || (1 / 0 == 0)) { conout(\"ok\"); } conout(\"survived\");",
        "ok\nsurvived\n"
    );

    expectError(
        "&& rejects a non-bool left operand",
        "if (1 && true) { }",
        "'&&' requires a bool, got int"
    );

    expectError(
        "&& rejects a non-bool right operand",
        "if (true && \"s\") { }",
        "'&&' requires a bool, got string"
    );

    expectError(
        "|| rejects a non-bool operand",
        "if (false || 2) { }",
        "'||' requires a bool, got int"
    );
}

void testReassignment() {
    std::cout << "reassignment\n";

    expectOutput(
        "a variable can be reassigned",
        "var int: x = 5; x = 7; conout(x);",
        "7\n"
    );

    expectOutput(
        "reassignment is visible in a block",
        "var int: x = 1; if (true) { x = 99; } conout(x);",
        "99\n"
    );

    expectOutput(
        "an inner declaration shadows, so the outer value is untouched",
        "var int: x = 1; if (true) { var int: x = 2; x = 3; conout(x); } conout(x);",
        "3\n1\n"
    );

    expectOutput(
        "a reassignment can feed a comparison",
        "var int: n = 1; n = n + 4; if (n == 5) { conout(\"five\"); }",
        "five\n"
    );

    expectError(
        "reassigning a wrongly typed value is rejected",
        "var int: x = 5; x = \"no\";",
        "Type mismatch in assignment to 'x'"
    );

    expectError(
        "assigning to an undefined variable is rejected",
        "ghost = 1;",
        "Undefined variable: ghost"
    );

    expectError(
        "a missing ';' is a syntax error",
        "var int: x = 1; x = 2 conout(x);",
        "Expected ';' after assignment"
    );

    expectOutput(
        "== is still a comparison, not an assignment",
        "var int: x = 2; if (x == 2) { conout(\"two\"); }",
        "two\n"
    );
}

void testAnyReassignment() {
    std::cout << "reassigning any variables\n";

    expectOutput(
        "an any variable may change type",
        "var any: v = 5; v = \"five\"; conout(v is(type: string));",
        "true\n"
    );

    expectOutput(
        "an any variable may switch back and forth",
        "var any: v = 5;"
        " conout(v is(type: int));"
        " v = 2.5;"
        " conout(v is(type: float));"
        " v = true;"
        " conout(v is(type: bool));",
        "true\ntrue\ntrue\n"
    );

    expectOutput(
        "a reassigned any keeps the new value's arithmetic type",
        "var any: v = 5; v = 7 / 2; conout(v);",
        "3.5\n"
    );

    expectOutput(
        "&& guards a division using a type check",
        "var any: divisor = 2; divisor = 0;"
        " if (divisor is(type: int) && divisor != 0) { conout(1 / divisor); } else { conout(\"skipped\"); }",
        "skipped\n"
    );
}

} // namespace

int main() {
    testConin();
    testConinTyped();
    testFunctions();
    testFunctionScope();
    testLogicalOperators();
    testReassignment();
    testAnyReassignment();
    testIfStatements();
    testTypeChecksInConditions();
    testConditionsMustBeBoolean();
    testScoping();
    testSyntaxErrors();
    testScopeReleasedAfterError();
    testAssignReachesEnclosingScope();
    testIntegerArithmetic();

    std::cout << "\n"
              << (checks - failures) << "/" << checks << " checks passed\n";

    if (failures != 0) {
        std::cout << failures << " FAILED\n";
        return 1;
    }

    return 0;
}
