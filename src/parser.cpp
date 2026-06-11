#include "../include/parser.hpp"
#include <iostream>
#include <stdexcept>

Parser::Parser(const std::vector<token>& tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
    if (match({tokenType::LET})) return varDeclaration();
    return statement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    token name = consume(tokenType::IDENTIFIER, "Expected variable name.");
    consume(tokenType::COLON, "Expected ':' after variable name.");
    
    if (!match({tokenType::NUM_TYPE, tokenType::BOOL})) {
        throw std::runtime_error("Expected variable type (num or bool).");
    }
    tokenType type = previous().type;

    consume(tokenType::EQUAL, "Expected '=' after variable type.");
    std::unique_ptr<Expr> initializer = expression();
    
    consume(tokenType::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_unique<VarDeclStmt>(name, type, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({tokenType::IF})) return ifStatement();
    if (match({tokenType::READ})) return readStatement();
    if (match({tokenType::ERE})) return whileStatement();
    if (match({tokenType::ECHO})) return printStatement();
    if (match({tokenType::L_BRACE})) return std::make_unique<BlockStmt>(block());
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(tokenType::L_PAREN, "Expected '(' after 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(tokenType::R_PAREN, "Expected ')' after if condition.");

    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;
    
    // check for else block
    if (match({tokenType::ELSE})) {
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::readStatement() {
    token name = consume(tokenType::IDENTIFIER, "Expected variable target name for read statement input.");
    consume(tokenType::SEMICOLON, "Expected ';' after read destination identifier.");
    return std::make_unique<ReadStmt>(name);
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(tokenType::L_PAREN, "Expected '(' after 'ere'.");
    std::unique_ptr<Expr> condition = expression();
    consume(tokenType::R_PAREN, "Expected ')' after loop condition.");
    
    std::unique_ptr<Stmt> body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(tokenType::R_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(tokenType::R_BRACE, "Expected '}' after code block.");
    return statements;
}

std::unique_ptr<Stmt> Parser::printStatement() {
    std::unique_ptr<Expr> value = expression();
    consume(tokenType::SEMICOLON, "Expected ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    consume(tokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = equality();
    if (match({tokenType::EQUAL})) {
        std::unique_ptr<Expr> value = assignment();
        if (VariableExpr* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            token name = varExpr->name;
            return std::make_unique<AssignExpr>(name, std::move(value));
        }
        throw std::runtime_error("Invalid assignment target.");
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();
    while (match({tokenType::EQUAL_EQUAL, tokenType::NOT_EQUAL})) {
        token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();
    while (match({tokenType::GREATER, tokenType::GREATER_EQUAL, tokenType::LESS, tokenType::LESS_EQUAL})) {
        token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();
    while (match({tokenType::MINUS, tokenType::PLUS})) {
        token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = primary();
    while (match({tokenType::SLASH, tokenType::STAR})) {
        token op = previous();
        std::unique_ptr<Expr> right = primary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({tokenType::LIE, tokenType::TRUTH, tokenType::NUM_LITERAL})) {
        return std::make_unique<LiteralExpr>(previous());
    }
    if (match({tokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }
    if (match({tokenType::L_PAREN})) {
        std::unique_ptr<Expr> expr = expression();
        consume(tokenType::R_PAREN, "Expected ')' after expression.");
        return expr;
    }
    throw std::runtime_error("Expected expression.");
}


//Functions for navigation
bool Parser::isAtEnd() { return peek().type == tokenType::END_OF_FILE; }
token Parser::peek() { return tokens[current]; }
token Parser::previous() { return tokens[current - 1]; }
token Parser::advance() { if (!isAtEnd()) current++; return previous(); }

bool Parser::check(tokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::vector<tokenType> types) {
    for (tokenType type : types) {
        if (check(type)) { advance(); return true; }
    }
    return false;
}

token Parser::consume(tokenType type, std::string message) {
    if (check(type)) return advance();
    std::cerr << "Syntax Error at line " << peek().line << ": " << message << std::endl;
    throw std::runtime_error(message);
}