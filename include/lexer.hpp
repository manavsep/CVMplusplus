#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <map>

enum class tokenType {
    L_PAREN, R_PAREN, COMMA, SEMICOLON, L_BRACE, R_BRACE, COLON, //punctuations
    PLUS, MINUS, STAR, SLASH, //math operators
    EQUAL, NOT_EQUAL, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL, EQUAL_EQUAL, //comparison and assignment
    IDENTIFIER, NUM_LITERAL, //identifiers and literals
    ECHO, READ, IF, ELSE, ERE, TRUTH, LIE, LET,BOOL,NUM_TYPE, //keywords
    END_OF_FILE //EOF
};

struct token{
    tokenType type;
    std::string value;
    int line;
    //constructor
    token(tokenType type, const std::string&value, int line)
        : type(type), value(value), line(line) {}
};

class Lexer{
public:
    Lexer(const std::string&source);
    std::vector<token> tokenize();

private:
    std::string source;
    std::vector<token> tokenList;
    int start=0;
    int current=0;
    int line=1;

    static std::map<std::string, tokenType> keywords;

    bool isAtEnd();
    char advance();
    bool match(char expected);
    char peek();
    void scanToken();
    void identifier();
    void number();
    void addToken(tokenType type);
    void addToken(tokenType type, std::string text); //for numbers and identifiers
};
#endif