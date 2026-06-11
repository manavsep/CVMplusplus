#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>
#include <string>
#include <cstdint>

using Value = int;

enum class OpCode : uint8_t {
    OP_CONSTANT,          // Pushes a literal value onto the execution stack
    OP_ADD,               // Binary +
    OP_SUBTRACT,          // Binary -
    OP_MULTIPLY,          // Binary *
    OP_DIVIDE,            // Binary /
    OP_EQUAL,             // Binary ==
    OP_GREATER,           // Binary >
    OP_LESS,              // Binary <
    OP_PRINT,             // Executes console echo commands
    OP_READ,              // Halts VM to intercept terminal input
    OP_DEFINE_GLOBAL,     // Declares and registers a global variable slot
    OP_GET_GLOBAL,        // Retrieves variable payload by string identity index
    OP_SET_GLOBAL,        // Overwrites variable payload by string identity index
    OP_POP,               // Cleanly discards top value from expression execution stack
    OP_JUMP,              // Unconditional forward jump modification
    OP_JUMP_IF_FALSE,     // Conditional forward jump modification
    OP_LOOP,              // Backward loop address modification
    OP_RETURN             // For Termination
};

class Chunk {
public:
    std::vector<uint8_t> code;           // Bytecode sequence array
    std::vector<int> lines;              // Line numbers tracking for runtime debugging
    std::vector<Value> constants;        // Number/Bool literal constant data pool
    std::vector<std::string> identifiers;// Variable name identities

    void writeChunk(uint8_t byte, int line);
    int addConstant(Value value);
    int addIdentifier(const std::string& name);
};

#endif