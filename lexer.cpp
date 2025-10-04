#include <cctype>
#include <sstream>
#include <unordered_set>
#include <iostream>

static const std::unordered_set<std::string> rust_keywords = {
"as","break","const","continue","crate","else","enum","extern","false",
"fn","for","if","impl","in","let","loop","match","mod","move","mut",
"pub","ref","return","self","Self","static","struct","super","trait","true",
"type","unsafe","use","where","while","async","await","dyn","abstract","become",
"box","do","final","macro","override","priv","try","typeof","unsized","virtual",
"yield"
};


std::string tokenTypeToString(TokenType t) {
	static std::unordered_map<TokenType, std::string> names = {
	{TokenType::IDENTIFIER,"IDENTIFIER"},{TokenType::KEYWORD,"KEYWORD"},
	{TokenType::DEC_INT,"DEC_INT"},{TokenType::HEX_INT,"HEX_INT"},
	{TokenType::FLOAT,"FLOAT"},{TokenType::STRING_LITERAL,"STRING_LITERAL"},
	{TokenType::CHAR_LITERAL,"CHAR_LITERAL"},{TokenType::DIRECTIVE,"DIRECTIVE"},
	{TokenType::MACRO,"MACRO"},{TokenType::COMMENT,"COMMENT"},
	{TokenType::OPERATOR,"OPERATOR"},{TokenType::PUNCTUATOR,"PUNCTUATOR"},
	{TokenType::NUMBER_UNKNOWN,"NUMBER_UNKNOWN"},{TokenType::UNKNOWN,"UNKNOWN"}
	};
	return names[t];
}




bool Lexer::isIdentStart(char c) { return std::isalpha((unsigned char)c) || c == '_'; }
bool Lexer::isIdentPart(char c) { return std::isalnum((unsigned char)c) || c == '_'; }
void Lexer::skipWhitespace() { while (pos < n && std::isspace((unsigned char)input[pos])) pos++; }


std::vector<Token> Lexer::tokenize() {
	std::vector<Token> tokens;
	while (pos < n) {
		skipWhitespace();
		if (pos >= n) break;
		char c = input[pos];
		Token t;
		if (isIdentStart(c)) t = readIdentifierOrKeyword();
		else if (std::isdigit((unsigned char)c)) t = readNumber();
		else if (c == '\"') t = readString();
		else if (c == '\'') t = readChar();
		else if (c == '/' && pos + 1 < n && (input[pos + 1] == '/' || input[pos + 1] == '*')) t = readComment();
		else if (c == '#') t = readDirective();
		else t = readOperatorOrPunct();
		tokens.push_back(t);
	}
	return tokens;
}

Token Lexer::readIdentifierOrKeyword() {
	size_t start = pos; pos++;
	while (pos < n && isIdentPart(input[pos])) pos++;
	std::string lex = input.substr(start, pos - start);
	// macro detection
	size_t temp = pos; while (temp < n && std::isspace((unsigned char)input[temp])) temp++;
	if (temp < n && input[temp] == '!') { pos = temp + 1; return { lex + "!", TokenType::MACRO }; }
	if (rust_keywords.count(lex)) return { lex, TokenType::KEYWORD };
	return { lex, TokenType::IDENTIFIER };
}


Token Lexer::readNumber() {
	size_t start = pos; bool hex = false, fl = false;
	if (input[pos] == '0' && pos + 1 < n && (input[pos + 1] == 'x' || input[pos + 1] == 'X')) {
		pos += 2; hex = true; while (pos < n && std::isxdigit((unsigned char)input[pos])) pos++; return { input.substr(start,pos - start),TokenType::HEX_INT };
	}
	while (pos < n && std::isdigit((unsigned char)input[pos])) pos++;
	if (pos < n && input[pos] == '.' && pos + 1 < n && std::isdigit((unsigned char)input[pos + 1])) {
		fl = true; pos++; while (pos < n && std::isdigit((unsigned char)input[pos])) pos++;
	}
	if (pos < n && (input[pos] == 'e' || input[pos] == 'E')) {
		fl = true; pos++; if (pos < n && (input[pos] == '+' || input[pos] == '-')) pos++; while (pos < n && std::isdigit((unsigned char)input[pos])) pos++;
	}
	return { input.substr(start,pos - start), fl ? TokenType::FLOAT : TokenType::DEC_INT };
}
Token Lexer::readString() {
	size_t start = pos++; while (pos < n) { if (input[pos] == '\\') { pos += 2; continue; } if (input[pos] == '\"') { pos++; break; } pos++; }
	return { input.substr(start,pos - start),TokenType::STRING_LITERAL };
}


Token Lexer::readChar() {
	size_t start = pos++; while (pos < n) { if (input[pos] == '\\') { pos += 2; continue; } if (input[pos] == '\'') { pos++; break; } pos++; }
	return { input.substr(start,pos - start),TokenType::CHAR_LITERAL };
}


Token Lexer::readComment() {
	size_t start = pos;
	if (input[pos + 1] == '/') { pos += 2; while (pos < n && input[pos] != '\n') pos++; }
	else { pos += 2; while (pos + 1 < n && !(input[pos] == '*' && input[pos + 1] == '/')) pos++; pos += 2; }
	return { input.substr(start,pos - start),TokenType::COMMENT };
}


Token Lexer::readDirective() {
	size_t start = pos; pos++;
	if (pos < n && input[pos] == '[') { int depth = 1; pos++; while (pos < n && depth>0) { if (input[pos] == '[')depth++; else if (input[pos] == ']')depth--; pos++; } }
	else { while (pos < n && input[pos] != '\n') pos++; }
	return { input.substr(start,pos - start),TokenType::DIRECTIVE };
}

Token Lexer::readOperatorOrPunct() {
	std::string ops = "+-*/%=<>!&|^~?:"; std::string punct = "()[]{};,.#@`$";
	size_t start = pos;
	std::vector<std::string> multi = { "::","->","=>","==","!=","<=",">=","&&","||","..","...","+=","-=","*=","/=","%=","<<=",">>=","<<",">>" };
	for (const auto& m : multi) { if (pos + m.size() <= n && input.substr(pos, m.size()) == m) { pos += m.size(); return { m,TokenType::OPERATOR }; } }
	char c = input[pos++];
	if (ops.find(c) != std::string::npos) return { std::string(1,c),TokenType::OPERATOR };
	if (punct.find(c) != std::string::npos) return { std::string(1,c),TokenType::PUNCTUATOR };
	return { std::string(1,c),TokenType::UNKNOWN };
}