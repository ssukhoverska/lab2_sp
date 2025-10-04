#include "lexer.h"
#include <cctype>
#include <unordered_set>
#include <iostream>

// --- Набір ключових слів Rust ---
static const std::unordered_set<std::string> keywords = {
    "fn", "let", "mut", "if", "else", "for", "while", "loop", "break", "continue",
    "return", "match", "struct", "enum", "impl", "trait", "use", "mod",
    "pub", "const", "static", "as", "in", "crate", "super", "self",
    "true", "false", "type", "where", "move", "ref", "unsafe", "dyn",
    "extern", "macro_rules", "async", "await", "yield"
};

// --- Перетворення типу токена у рядок ---
std::string tokenTypeToString(TokenType type) {
    switch (type) {
    case TokenType::IDENTIFIER: return "IDENTIFIER";
    case TokenType::KEYWORD: return "KEYWORD";
    case TokenType::DEC_INT: return "DEC_INT";
    case TokenType::HEX_INT: return "HEX_INT";
    case TokenType::FLOAT: return "FLOAT";
    case TokenType::STRING_LITERAL: return "STRING_LITERAL";
    case TokenType::CHAR_LITERAL: return "CHAR_LITERAL";
    case TokenType::DIRECTIVE: return "DIRECTIVE";
    case TokenType::MACRO: return "MACRO";
    case TokenType::COMMENT: return "COMMENT";
    case TokenType::OPERATOR: return "OPERATOR";
    case TokenType::PUNCTUATOR: return "PUNCTUATOR";
    case TokenType::UNKNOWN: return "UNKNOWN";
    default: return "UNKNOWN";
    }
}

// --- Конструктор ---
Lexer::Lexer(const std::string& input) : input(input), pos(0), n(input.size()) {}

// --- Перевірка символів ---
bool Lexer::isIdentStart(char c) {
    return std::isalpha(c) || c == '_' || (unsigned char)c >= 128;
}

bool Lexer::isIdentPart(char c) {
    return std::isalnum(c) || c == '_' || (unsigned char)c >= 128;
}

void Lexer::skipWhitespace() {
    while (pos < n && std::isspace((unsigned char)input[pos])) pos++;
}

// --- Основна функція токенізації ---
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < n) {
        skipWhitespace();
        if (pos >= n) break;

        char c = input[pos];

        if (isIdentStart(c)) {
            tokens.push_back(readIdentifierOrKeyword());
        }
        else if (std::isdigit(c)) {
            tokens.push_back(readNumber());
        }
        else if (c == '"') {
            tokens.push_back(readString());
        }
        else if (c == '\'') {
            tokens.push_back(readChar());
        }
        else if (c == '/' && pos + 1 < n && (input[pos + 1] == '/' || input[pos + 1] == '*')) {
            tokens.push_back(readComment());
        }
        else if (c == '#') {
            tokens.push_back(readDirective());
        }
        else if (std::ispunct((unsigned char)c)) {
            tokens.push_back(readOperatorOrPunct());
        }
        else {
            std::string lex(1, c);
            tokens.push_back({ lex, TokenType::UNKNOWN });
            pos++;
        }
    }

    return tokens;
}

// --- Зчитування ідентифікатора або ключового слова ---
Token Lexer::readIdentifierOrKeyword() {
    size_t start = pos;
    while (pos < n && isIdentPart(input[pos])) pos++;
    std::string lex = input.substr(start, pos - start);

    if (keywords.count(lex)) return { lex, TokenType::KEYWORD };
    if (lex.size() > 1 && lex.back() == '!') return { lex, TokenType::MACRO };
    return { lex, TokenType::IDENTIFIER };
}

// --- Зчитування числа ---
Token Lexer::readNumber() {
    size_t start = pos;
    bool isFloat = false;

    if (input[pos] == '0' && (pos + 1 < n) && (input[pos + 1] == 'x' || input[pos + 1] == 'X')) {
        pos += 2;
        while (pos < n && std::isxdigit((unsigned char)input[pos])) pos++;
        return { input.substr(start, pos - start), TokenType::HEX_INT };
    }

    while (pos < n && (std::isdigit((unsigned char)input[pos]) || input[pos] == '.')) {
        if (input[pos] == '.') isFloat = true;
        pos++;
    }

    return { input.substr(start, pos - start), isFloat ? TokenType::FLOAT : TokenType::DEC_INT };
}

// --- Зчитування рядкового літералу ---
Token Lexer::readString() {
    size_t start = pos++;
    while (pos < n && input[pos] != '"') {
        if (input[pos] == '\\' && pos + 1 < n) pos++;
        pos++;
    }
    if (pos < n) pos++; // закриваюча лапка
    return { input.substr(start, pos - start), TokenType::STRING_LITERAL };
}

// --- Зчитування символьного літералу ---
Token Lexer::readChar() {
    size_t start = pos++;
    while (pos < n && input[pos] != '\'') {
        if (input[pos] == '\\' && pos + 1 < n) pos++;
        pos++;
    }
    if (pos < n) pos++;
    return { input.substr(start, pos - start), TokenType::CHAR_LITERAL };
}

// --- Зчитування коментарів ---
Token Lexer::readComment() {
    size_t start = pos;
    if (input[pos + 1] == '/') {
        pos += 2;
        while (pos < n && input[pos] != '\n') pos++;
    }
    else {
        pos += 2;
        while (pos + 1 < n && !(input[pos] == '*' && input[pos + 1] == '/')) pos++;
        if (pos + 1 < n) pos += 2;
    }
    return { input.substr(start, pos - start), TokenType::COMMENT };
}

// --- Зчитування директив препроцесора ---
Token Lexer::readDirective() {
    size_t start = pos;
    while (pos < n && input[pos] != '\n') pos++;
    return { input.substr(start, pos - start), TokenType::DIRECTIVE };
}

// --- Зчитування операторів або розділових знаків ---
Token Lexer::readOperatorOrPunct() {
    static const std::unordered_set<std::string> operators = {
        "+", "-", "*", "/", "%", "=", "==", "!=", ">", "<", ">=", "<=",
        "&&", "||", "!", "&", "|", "^", ">>", "<<", "->", "::", "+=", "-=",
        "*=", "/=", "%="
    };

    static const std::unordered_set<char> punctuators = {
        '(', ')', '{', '}', '[', ']', ',', ';', ':', '.'
    };

    size_t start = pos;
    std::string op(1, input[pos]);
    pos++;

    if (pos < n) {
        std::string two = op + input[pos];
        if (operators.count(two)) {
            pos++;
            return { two, TokenType::OPERATOR };
        }
    }

    if (operators.count(op)) return { op, TokenType::OPERATOR };
    if (punctuators.count(op[0])) return { op, TokenType::PUNCTUATOR };
    return { op, TokenType::UNKNOWN };
}
