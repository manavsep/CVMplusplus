#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <vector>
#include <memory>
#include <cstdint>
#include "parser.hpp"
#include "chunk.hpp"

class Compiler {
private:
    Chunk chunk;

    void compileStatement(const Stmt* stmt);
    void compileExpression(const Expr* expr);

    // bytecode emission systems
    void emitByte(uint8_t byte, int line);
    void emitBytes(uint8_t byte1, uint8_t byte2, int line);
    void emitConstant(Value value, int line);
    
    // Structure flow systems
    int emitJump(uint8_t instruction, int line);
    void patchJump(int offset);
    void emitLoop(int loopStart, int line);

public:
    Compiler() = default;
    Chunk compile(const std::vector<std::unique_ptr<Stmt>>& ast);
};

#endif