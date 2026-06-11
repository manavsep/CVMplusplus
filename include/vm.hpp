#ifndef VM_HPP
#define VM_HPP

#include <vector>
#include <unordered_map>
#include "chunk.hpp"

class VM {
public:
    enum class InterpretResult {
        INTERPRET_OK,
        INTERPRET_COMPILE_ERROR,
        INTERPRET_RUNTIME_ERROR
    };
private:
    Chunk chunk;
    size_t ip = 0;                  // Instruction Pointer tracking execution position
    std::vector<Value> stack;       // The data stack for tracking operations
    std::unordered_map<uint8_t, Value> globals; // Memory maps for active variable scopes

    // Internal VM stack manipulation utilities
    void push(Value value);
    Value pop();
    Value peek(int distance);
    
    // Bytecode interpretation utilities
    uint8_t readByte();
    uint16_t readShort();

    // Core execution loop processing switchboard
    InterpretResult run();

public:
    InterpretResult interpret(const Chunk& chunk);
};

#endif