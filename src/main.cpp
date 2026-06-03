#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "../include/lexer.hpp"

void run(const std::string& source) {
    Lexer lexer(source);
    std::vector<token> tokens = lexer.tokenize();

    // Print the tokens to see if it worked
    std::cout << "--- Tokens Found ---" << std::endl;
    for (const auto& t : tokens) {
        std::cout << "[Line " << t.line << "] Type: " << (int)t.type 
                  << " | Value: '" << t.value << "'" << std::endl;
    }
}

void runFile(const char*path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "(ERROR) Could not open file: " << path << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());
}

int main(int argc, char*argv[]) {
    if (argc < 2) {
        std::cout << "Usage: cvm [script_path]" << std::endl;
        return 64;
    }

    runFile(argv[1]);
    return 0;
}