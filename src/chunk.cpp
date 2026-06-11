#include "../include/chunk.hpp"

void Chunk::writeChunk(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
}

int Chunk::addConstant(Value value) {
    constants.push_back(value);
    return constants.size() - 1; // Return index of the added constant
}

int Chunk::addIdentifier(const std::string& name) {
    // Check if the identifier already exists to save space layout overhead
    for (size_t i = 0; i < identifiers.size(); ++i) {
        if (identifiers[i] == name) {
            return i;
        }
    }
    identifiers.push_back(name);
    return identifiers.size() - 1;
}