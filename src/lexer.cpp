#include "../include/lexer.hpp"
#include <iostream>
std::map<std::string, tokenType> Lexer::keywords = {
    {"let", tokenType::LET},
    {"echo", tokenType::ECHO},
    {"read", tokenType::READ},
    {"if", tokenType::IF},
    {"else", tokenType::ELSE},
    {"ere", tokenType::ERE},
    {"truth", tokenType::TRUTH},
    {"lie", tokenType::LIE},
    {"bool", tokenType::BOOL},
    {"num", tokenType::NUM}
};

Lexer::Lexer(const std::string&source) : source(source) {}

std::vector<token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokenList.push_back(token(tokenType::END_OF_FILE, "", line));
    return tokenList;
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(tokenType::L_PAREN); break;
        case ')': addToken(tokenType::R_PAREN); break;
        case '{': addToken(tokenType::L_BRACE); break;
        case '}': addToken(tokenType::R_BRACE); break;
        case ',': addToken(tokenType::COMMA); break;
        case '-': addToken(tokenType::MINUS); break;
        case '+': addToken(tokenType::PLUS); break;
        case ';': addToken(tokenType::SEMICOLON); break;
        case '*': addToken(tokenType::STAR); break;
        case '/': addToken(tokenType::SLASH); break;
        case '!':
            if (match('=')) addToken(tokenType::NOT_EQUAL);
            else std::cerr << "Line " << line << ": Unexpected character '!'." << std::endl;
            break;
        case '=':
            addToken(match('=') ? tokenType::EQUAL_EQUAL : tokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? tokenType::LESS_EQUAL : tokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? tokenType::GREATER_EQUAL : tokenType::GREATER);
            break;
        case ':': addToken(tokenType::COLON); break;
        case ' ':
        case '\r':
        case '\t': break; // we will ignore all whitespaces
        case '\n': line++; break;
        default:
            if (isdigit(c)) {
                number();
            } else if (isalpha(c) || c == '_') {
                identifier();
            } else {
                std::cerr << "Line " << line << ": Unexpected character. Please fix." << std::endl;
            }
            break;
    }
}

void Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_') advance();
    std::string text = source.substr(start, current - start);
    tokenType type = tokenType::IDENTIFIER;
    
    if (keywords.count(text)) {
        type = keywords.at(text); //if the identifier is a keyword, we will change its type to the corresponding keyword type
    }
    addToken(type, text);
}

void Lexer::number() {
    while (isdigit(peek())) advance();
    addToken(tokenType::NUM, source.substr(start, current - start));
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    return true;
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::advance() {
    return source[current++];
}

bool Lexer::isAtEnd() {
    return current >= source.length();
}

void Lexer::addToken(tokenType type) {
    addToken(type, source.substr(start, current - start));
}

void Lexer::addToken(tokenType type, std::string text) {
    tokenList.push_back(token(type, text, line));
}