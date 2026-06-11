#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <cstdlib> // For exit codes
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/compiler.hpp"
#include "../include/vm.hpp"

void run(const std::string& source) {
    Lexer lexer(source);
    std::vector<token> tokens = lexer.tokenize();

    // PHASE 1: LEXER
    std::cout << "--- Tokens Found ---" << std::endl;
    for (const auto& t : tokens) {
        std::cout << "[Line " << t.line << "] Type: " << (int)t.type 
                  << " | Value: '" << t.value << "'" << std::endl;
    }

    std::cout << "\n--- Running Parser ---" << std::endl;
    try {
        //PHASE 2: PARSING
        Parser parser(tokens);
        std::vector<std::unique_ptr<Stmt>> ast = parser.parse();
        
        std::cout << "SUCCESS: Parsing complete!" << std::endl;
        std::cout << "Built an Abstract Syntax Tree with " << ast.size() << " top-level statements." << std::endl;
        
        // PHASE 3: COMPILATION
        std::cout << "\n--- Running Compiler ---" << std::endl;
        Compiler compiler;
        Chunk chunk = compiler.compile(ast);
        std::cout << "SUCCESS: Compilation complete!" << std::endl;
        std::cout << "Generated " << chunk.code.size() << " bytes of executable bytecode." << std::endl;

        // PHASE 4: VIRTUAL MACHINE EXECUTION
        std::cout << "\n--- Executing Program via Virtual Machine ---" << std::endl;
        VM vm;
        VM::InterpretResult result = vm.interpret(chunk);

        std::cout << "---------------------------------------------" << std::endl;
        if (result == VM::InterpretResult::INTERPRET_OK) {
            std::cout << "SUCCESS: Program executed without runtime errors." << std::endl;
        } else if (result == VM::InterpretResult::INTERPRET_RUNTIME_ERROR) {
            std::cerr << "FAILURE: Execution caught a critical runtime exception." << std::endl;
            std::exit(70); // Software error code for internal runtime failure
        }
        
    } catch (const std::exception& e) {
        std::cerr << "\nPROCESS ABORTED: " << e.what() << std::endl;
        std::exit(65); // Misuse of shell tools / data formatting failure code
    }
}

void runFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "(ERROR) Could not open file: " << path << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: cvm [script_path]" << std::endl;
        return 64; // Command line usage error code
    }

    runFile(argv[1]);
    return 0;
}