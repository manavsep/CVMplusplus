#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>
#include <memory>
#include "lexer.hpp"

//Expressions and the types of expressions

class Expr {
public:
    virtual ~Expr() = default; //virtual so that the actual expression type's destructor is called
};

class LiteralExpr : public Expr {
public:
    token value;
    LiteralExpr(token value) : value(value) {}
};

class VariableExpr : public Expr {
public:
    token name;
    VariableExpr(token name) : name(name) {}
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    //we transfer ownership of the expressions from their original pointers to left and right pointers
};

class AssignExpr : public Expr {
public:
    token name;
    std::unique_ptr<Expr> value;
    AssignExpr(token name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
};

//Statements and their kinds
class Stmt {
public:
    virtual ~Stmt() = default;
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // Can be nullptr if there is no else block
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
};

class ReadStmt : public Stmt {
public:
    token name;
    ReadStmt(token name) : name(name) {}
};

class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    ExpressionStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
};

class PrintStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
};

class VarDeclStmt : public Stmt {
public:
    token name;
    tokenType varType; // NUM_TYPE or BOOL
    std::unique_ptr<Expr> initializer;
    VarDeclStmt(token name, tokenType varType, std::unique_ptr<Expr> initializer)
        : name(name), varType(varType), initializer(std::move(initializer)) {}
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements) : statements(std::move(statements)) {}
};

class WhileStmt : public Stmt { //basically the ere loop
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}
};

class Parser {
private:
    std::vector<token> tokens;
    int current = 0;

    //functions for navigating
    bool isAtEnd();
    token peek();
    token previous();
    token advance();
    bool check(tokenType type);
    bool match(std::vector<tokenType> types);
    token consume(tokenType type, std::string message);

    //functions that deal with statements
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> readStatement();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::vector<std::unique_ptr<Stmt>> block();
    std::unique_ptr<Stmt> expressionStatement();

    //functions that deal with expressions
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> primary();

public:
    Parser(const std::vector<token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();
};

#endif

