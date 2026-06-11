#include "../include/vm.hpp"
#include <iostream>
#include <stdexcept>

void VM::push(Value value) {
    stack.push_back(value);
}

Value VM::pop() {
    if (stack.empty()) {
        throw std::runtime_error("Runtime Execution Error: Stack underflow anomaly.");
    }
    Value back = stack.back();
    stack.pop_back();
    return back;
}

Value VM::peek(int distance) {
    if (stack.size() <= static_cast<size_t>(distance)) {
        throw std::runtime_error("Runtime Execution Error: Invalid stack inspection depth.");
    }
    return stack[stack.size() - 1 - distance];
}

uint8_t VM::readByte() {
    return chunk.code[ip++];
}

uint16_t VM::readShort() {
    ip += 2;
    return (static_cast<uint16_t>(chunk.code[ip - 2]) << 8) | chunk.code[ip - 1];
}

VM::InterpretResult VM::interpret(const Chunk& inputChunk) {
    this->chunk = inputChunk;
    this->ip = 0;
    this->stack.clear();
    return run();
}

VM::InterpretResult VM::run() {
    while (true) {
        uint8_t instruction = readByte();
        switch (static_cast<OpCode>(instruction)) {
            
            case OpCode::OP_CONSTANT: {
                uint8_t constantIndex = readByte();
                push(chunk.constants[constantIndex]);
                break;
            }
            
            case OpCode::OP_ADD: {
                Value b = pop();
                Value a = pop();
                push(a + b);
                break;
            }
            
            case OpCode::OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                push(a - b);
                break;
            }
            
            case OpCode::OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                push(a * b);
                break;
            }
            
            case OpCode::OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                if (b == 0) {
                    std::cerr << "Runtime Error at line " << chunk.lines[ip - 1] << ": Division by zero is prohibited.\n";
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(a / b);
                break;
            }
            
            case OpCode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(a == b ? 1 : 0);
                break;
            }
            
            case OpCode::OP_GREATER: {
                Value b = pop();
                Value a = pop();
                push(a > b ? 1 : 0);
                break;
            }
            
            case OpCode::OP_LESS: {
                Value b = pop();
                Value a = pop();
                push(a < b ? 1 : 0);
                break;
            }
            
            case OpCode::OP_PRINT: {
                Value val = pop();
                std::cout << val << "\n";
                break;
            }
            
            case OpCode::OP_READ: {
                int userInput;
                std::cout << "? "; // Terminal input indicator prompt
                while (!(std::cin >> userInput)) {
                    std::cin.clear();
                    std::string discard;
                    std::getline(std::cin, discard);
                    std::cout << "Invalid type. Please enter a valid numerical input.\n? ";
                }
                push(userInput);
                break;
            }
            
            case OpCode::OP_DEFINE_GLOBAL: {
                uint8_t nameIndex = readByte();
                globals[nameIndex] = pop();
                break;
            }
            
            case OpCode::OP_GET_GLOBAL: {
                uint8_t nameIndex = readByte();
                if (globals.find(nameIndex) == globals.end()) {
                    std::cerr << "Runtime Error: Undefined variable usage index identity '" << (int)nameIndex << "'.\n";
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(globals[nameIndex]);
                break;
            }
            
            case OpCode::OP_SET_GLOBAL: {
                uint8_t nameIndex = readByte();
                if (globals.find(nameIndex) == globals.end()) {
                    std::cerr << "Runtime Error: Cannot assign to undefined symbol mapping assignment target.\n";
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                
                globals[nameIndex] = peek(0);
                break;
            }
            
            case OpCode::OP_POP: {
                pop();
                break;
            }
            
            case OpCode::OP_JUMP: {
                uint16_t offset = readShort();
                ip += offset;
                break;
            }
            
            case OpCode::OP_JUMP_IF_FALSE: {
                uint16_t offset = readShort();
                
                if (peek(0) == 0) ip += offset;
                break;
            }
            
            case OpCode::OP_LOOP: {
                uint16_t offset = readShort();
                ip -= offset;
                break;
            }
            
            case OpCode::OP_RETURN: {
                return InterpretResult::INTERPRET_OK;
            }
        }
    }
}