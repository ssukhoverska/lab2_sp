#ifndef LEXER_H
#define LEXER_H
#include <string>
#include <vector>
#include "token.h"

class Lexer {
public:
    explicit Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    std::string input;
    size_t pos, n;

    bool isIdentStart(char c);
    bool isIdentPart(char c);
    void skipWhitespace();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString();
    Token readChar();
    Token readComment();
    Token readDirective();
    Token readOperatorOrPunct();
};

#endif // LEXER_H
