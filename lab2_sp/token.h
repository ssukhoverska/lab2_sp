#ifndef TOKEN_H
#define TOKEN_H
#include <string>
#include <unordered_map>

enum class TokenType {
    IDENTIFIER,
    KEYWORD,
    DEC_INT,
    HEX_INT,
    FLOAT,
    STRING_LITERAL,
    CHAR_LITERAL,
    DIRECTIVE,
    MACRO,
    COMMENT,
    OPERATOR,
    PUNCTUATOR,
    NUMBER_UNKNOWN,
    UNKNOWN
};

struct Token {
    std::string lexeme;
    TokenType type;
};

std::string tokenTypeToString(TokenType type);

#endif // TOKEN_H
