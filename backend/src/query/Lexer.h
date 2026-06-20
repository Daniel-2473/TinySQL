#pragma once
#include "Token.h"
#include <string>
#include <vector>

class Lexer {
public:
    Lexer(const std::string& input);
    std::vector<Token> tokenize();

private:
    std::string input;
    int         pos = 0;

    char        current();
    char        peek();
    void        advance();
    void        skipWhitespace();

    Token       readIdentifierOrKeyword();
    Token       readStringLiteral();
    Token       readNumber();

    TokenType   classifyKeyword(const std::string& word);
};