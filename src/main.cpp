#include "Lexer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printHelp(const char* programName) {
    std::cout << "VECTx - A small statically typed scripting language\n\n";
    std::cout << "Usage: " << programName << " [script.vex]\n\n";
    std::cout << "If no script file is provided, this help message is shown.\n";
    std::cout << "Script files are VECTx source code with .vex extension (by convention).\n";
}

int runSource(const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        Program program = parser.parse();
        Interpreter interpreter;
        interpreter.execute(program);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [script.vex]\n";
        return 1;
    }

    if (argc == 2) {
        try {
            std::string source = readFile(argv[1]);
            return runSource(source);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
            return 1;
        }
    }

    // No file provided: show help
    printHelp(argv[0]);
    return 0;
}
