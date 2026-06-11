#include "../include/compiler.hpp"
#include <stdexcept>
#include <string>

Chunk Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& ast) {
    for (const auto& stmt : ast) {
        compileStatement(stmt.get());
    }
    // [TOKEN: END_OF_FILE] 
    emitByte(static_cast<uint8_t>(OpCode::OP_RETURN), 1); 
    return chunk;
}

void Compiler::compileStatement(const Stmt* stmt) {
    // [TOKENS: LET, COLON, NUM_TYPE, BOOL] 
    if (auto varDecl = dynamic_cast<const VarDeclStmt*>(stmt)) {
        compileExpression(varDecl->initializer.get());
        int nameIndex = chunk.addIdentifier(varDecl->name.value); 
        emitBytes(static_cast<uint8_t>(OpCode::OP_DEFINE_GLOBAL), static_cast<uint8_t>(nameIndex), varDecl->name.line);
    } 
    // [TOKEN: ECHO]
    else if (auto printStmt = dynamic_cast<const PrintStmt*>(stmt)) {
        compileExpression(printStmt->expression.get()); 
        emitByte(static_cast<uint8_t>(OpCode::OP_PRINT), 1);
    } 
    // [TOKEN: READ] 
    else if (auto readStmt = dynamic_cast<const ReadStmt*>(stmt)) {
        emitByte(static_cast<uint8_t>(OpCode::OP_READ), readStmt->name.line);
        int nameIndex = chunk.addIdentifier(readStmt->name.value);
        emitBytes(static_cast<uint8_t>(OpCode::OP_SET_GLOBAL), static_cast<uint8_t>(nameIndex), readStmt->name.line);
        emitByte(static_cast<uint8_t>(OpCode::OP_POP), readStmt->name.line); 
    }
    // [TOKENS: IF, ELSE] 
    else if (auto ifStmt = dynamic_cast<const IfStmt*>(stmt)) {
        compileExpression(ifStmt->condition.get());

        int thenJump = emitJump(static_cast<uint8_t>(OpCode::OP_JUMP_IF_FALSE), 1);
        emitByte(static_cast<uint8_t>(OpCode::OP_POP), 1); // Discard condition if execution is truth

        compileStatement(ifStmt->thenBranch.get());
        int elseJump = emitJump(static_cast<uint8_t>(OpCode::OP_JUMP), 1);

        patchJump(thenJump);
        emitByte(static_cast<uint8_t>(OpCode::OP_POP), 1); // Discard condition if execution is lie

        if (ifStmt->elseBranch != nullptr) {
            compileStatement(ifStmt->elseBranch.get());
        }
        patchJump(elseJump);
    }
    // [TOKEN: ERE] 
    else if (auto whileStmt = dynamic_cast<const WhileStmt*>(stmt)) {
        int loopStart = chunk.code.size(); 
        compileExpression(whileStmt->condition.get());
        
        int exitJump = emitJump(static_cast<uint8_t>(OpCode::OP_JUMP_IF_FALSE), 1);
        emitByte(static_cast<uint8_t>(OpCode::OP_POP), 1); 
        
        compileStatement(whileStmt->body.get());
        emitLoop(loopStart, 1);
        
        patchJump(exitJump);
        emitByte(static_cast<uint8_t>(OpCode::OP_POP), 1); 
    } 
    // [TOKEN: SEMICOLON] 
    else if (auto exprStmt = dynamic_cast<const ExpressionStmt*>(stmt)) {
        compileExpression(exprStmt->expression.get());
        emitByte(static_cast<uint8_t>(OpCode::OP_POP), 1); 
    } 
    // [TOKENS: L_BRACE, R_BRACE] 
    else if (auto blockStmt = dynamic_cast<const BlockStmt*>(stmt)) {
        for (const auto& nestedStmt : blockStmt->statements) {
            compileStatement(nestedStmt.get());
        }
    } 
    else {
        throw std::runtime_error("Compiler Error: Unknown Statement type structural fallback reached.");
    }
}

void Compiler::compileExpression(const Expr* expr) {
    // [TOKENS: NUM_LITERAL, TRUTH, LIE] 
    if (auto literal = dynamic_cast<const LiteralExpr*>(expr)) {
        if (literal->value.type == tokenType::NUM_LITERAL) {
            int value = std::stoi(literal->value.value);
            emitConstant(value, literal->value.line);
        } else if (literal->value.type == tokenType::TRUTH) {
            emitConstant(1, literal->value.line); 
        } else if (literal->value.type == tokenType::LIE) {
            emitConstant(0, literal->value.line); 
        }
        return;
    }

    // [TOKEN: IDENTIFIER]
    if (auto varExpr = dynamic_cast<const VariableExpr*>(expr)) {
        int nameIndex = chunk.addIdentifier(varExpr->name.value);
        emitBytes(static_cast<uint8_t>(OpCode::OP_GET_GLOBAL), static_cast<uint8_t>(nameIndex), varExpr->name.line);
        return;
    }

    // [TOKENS: IDENTIFIER + EQUAL]
    if (auto assignExpr = dynamic_cast<const AssignExpr*>(expr)) {
        compileExpression(assignExpr->value.get());
        int nameIndex = chunk.addIdentifier(assignExpr->name.value);
        emitBytes(static_cast<uint8_t>(OpCode::OP_SET_GLOBAL), static_cast<uint8_t>(nameIndex), assignExpr->name.line);
        return;
    }

    // [TOKENS: PLUS, MINUS, STAR, SLASH, EQUAL_EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL]
    if (auto binary = dynamic_cast<const BinaryExpr*>(expr)) {
        compileExpression(binary->left.get());
        compileExpression(binary->right.get());

        switch (binary->op.type) {
            case tokenType::PLUS:          emitByte(static_cast<uint8_t>(OpCode::OP_ADD), binary->op.line); break;
            case tokenType::MINUS:         emitByte(static_cast<uint8_t>(OpCode::OP_SUBTRACT), binary->op.line); break;
            case tokenType::STAR:          emitByte(static_cast<uint8_t>(OpCode::OP_MULTIPLY), binary->op.line); break;
            case tokenType::SLASH:         emitByte(static_cast<uint8_t>(OpCode::OP_DIVIDE), binary->op.line); break;
            case tokenType::EQUAL_EQUAL:   emitByte(static_cast<uint8_t>(OpCode::OP_EQUAL), binary->op.line); break;
            case tokenType::GREATER:       emitByte(static_cast<uint8_t>(OpCode::OP_GREATER), binary->op.line); break;
            case tokenType::LESS:          emitByte(static_cast<uint8_t>(OpCode::OP_LESS), binary->op.line); break;
            
            case tokenType::NOT_EQUAL:     
                emitByte(static_cast<uint8_t>(OpCode::OP_EQUAL), binary->op.line);
                emitConstant(0, binary->op.line); 
                emitByte(static_cast<uint8_t>(OpCode::OP_EQUAL), binary->op.line);
                break;

            case tokenType::GREATER_EQUAL: 
                emitByte(static_cast<uint8_t>(OpCode::OP_LESS), binary->op.line);
                emitConstant(0, binary->op.line); 
                emitByte(static_cast<uint8_t>(OpCode::OP_EQUAL), binary->op.line);
                break;

            case tokenType::LESS_EQUAL:    
                emitByte(static_cast<uint8_t>(OpCode::OP_GREATER), binary->op.line);
                emitConstant(0, binary->op.line); 
                emitByte(static_cast<uint8_t>(OpCode::OP_EQUAL), binary->op.line);
                break;
                
            default: throw std::runtime_error("Compiler Error: Expression mathematical token structural logic missing.");
        }
        return;
    }

    throw std::runtime_error("Compiler Error: Unknown Expression parsing variant structural layout encountered.");
}

void Compiler::emitByte(uint8_t byte, int line) {
    chunk.writeChunk(byte, line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2, int line) {
    emitByte(byte1, line);
    emitByte(byte2, line);
}

void Compiler::emitConstant(Value value, int line) {
    int index = chunk.addConstant(value);
    emitBytes(static_cast<uint8_t>(OpCode::OP_CONSTANT), static_cast<uint8_t>(index), line);
}

int Compiler::emitJump(uint8_t instruction, int line) {
    emitByte(instruction, line);
    emitByte(0xff, line); 
    emitByte(0xff, line); 
    return chunk.code.size() - 2; 
}

void Compiler::patchJump(int offset) {
    int jumpSize = chunk.code.size() - offset - 2;
    if (jumpSize > 65535) {
        throw std::runtime_error("Compiler Error: Cross-branch structural jump space size overflow constraint reached.");
    }
    chunk.code[offset] = (jumpSize >> 8) & 0xff;
    chunk.code[offset + 1] = jumpSize & 0xff;
}

void Compiler::emitLoop(int loopStart, int line) {
    emitByte(static_cast<uint8_t>(OpCode::OP_LOOP), line);
    int offset = chunk.code.size() - loopStart + 2;
    if (offset > 65535) throw std::runtime_error("Compiler Error: Loop space allocation bounds overflow layout error.");
    
    emitByte((offset >> 8) & 0xff, line);
    emitByte(offset & 0xff, line);
}