// query/Lexer.cpp
#include "Lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string& input) : input(input), pos(0) {}

// Devuelve el carácter actual sin avanzar
char Lexer::current() {
    if (pos >= input.size()) return '\0';
    return input[pos];
}

// Mira el siguiente carácter sin avanzar
char Lexer::peek() {
    if (pos + 1 >= input.size()) return '\0';
    return input[pos + 1];
}

// Avanza un carácter
void Lexer::advance() {
    pos++;
}

// Salta espacios, tabs y saltos de línea
void Lexer::skipWhitespace() {
    while (pos < input.size() && std::isspace(current()))
        advance();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < input.size()) {
        skipWhitespace();

        if (pos >= input.size()) break;

        char c = current();

        // Comentario de línea (//)
        if (c == '/' && peek() == '/') {
            while (pos < input.size() && current() != '\n')
                advance();
            continue;
        }

        // Identificador o palabra clave
        if (std::isalpha(c) || c == '_') {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }

        // Número
        if (std::isdigit(c)) {
            tokens.push_back(readNumber());
            continue;
        }

        // String entre comillas dobles
        if (c == '"') {
            tokens.push_back(readStringLiteral());
            continue;
        }

        // Símbolos de un solo carácter
        switch (c) {
            case '*': tokens.push_back({TokenType::STAR,      "*"}); break;
            case ',': tokens.push_back({TokenType::COMMA,     ","}); break;
            case ';': tokens.push_back({TokenType::SEMICOLON, ";"}); break;
            case '(': tokens.push_back({TokenType::LPAREN,    "("}); break;
            case ')': tokens.push_back({TokenType::RPAREN,    ")"}); break;
            case '=': tokens.push_back({TokenType::EQUALS,    "="}); break;
            case '>': tokens.push_back({TokenType::GREATER,   ">"}); break;
            case '<': tokens.push_back({TokenType::LESS,      "<"}); break;
            default:  tokens.push_back({TokenType::UNKNOWN,   std::string(1, c)}); break;
        }
        advance();
    }

    tokens.push_back({TokenType::END_OF_INPUT, ""});
    return tokens;
}

Token Lexer::readIdentifierOrKeyword() {
    std::string word;

    while (pos < input.size() && (std::isalnum(current()) || current() == '_')) {
        word += current();
        advance();
    }

    // Convertir a mayúsculas para comparar
    std::string upper = word;
    for (char& ch : upper) ch = std::toupper(ch);

    TokenType type = classifyKeyword(upper);

    // Si no es keyword, es un identificador (nombre de tabla, columna, etc.)
    if (type == TokenType::UNKNOWN)
        return {TokenType::IDENTIFIER, word};

    return {type, upper};
}

Token Lexer::readNumber() {
    std::string num;
    bool hasDecimal = false;

    while (pos < input.size() && (std::isdigit(current()) || current() == '.')) {
        if (current() == '.') {
            if (hasDecimal) break; // segundo punto, salir
            hasDecimal = true;
        }
        num += current();
        advance();
    }

    return {TokenType::NUMBER_LITERAL, num};
}

Token Lexer::readStringLiteral() {
    advance(); // salta la comilla de apertura
    std::string str;

    while (pos < input.size() && current() != '"') {
        str += current();
        advance();
    }
    advance(); // salta la comilla de cierre

    return {TokenType::STRING_LITERAL, str};
}

TokenType Lexer::classifyKeyword(const std::string& word) {
    if (word == "SELECT")   return TokenType::SELECT;
    if (word == "FROM")     return TokenType::FROM;
    if (word == "WHERE")    return TokenType::WHERE;
    if (word == "INSERT")   return TokenType::INSERT;
    if (word == "INTO")     return TokenType::INTO;
    if (word == "VALUES")   return TokenType::VALUES;
    if (word == "UPDATE")   return TokenType::UPDATE;
    if (word == "SET")      return TokenType::SET;
    if (word == "DELETE")   return TokenType::DELETE;
    if (word == "CREATE")   return TokenType::CREATE;
    if (word == "DROP")     return TokenType::DROP;
    if (word == "DATABASE") return TokenType::DATABASE;
    if (word == "TABLE")    return TokenType::TABLE;
    if (word == "INDEX")    return TokenType::INDEX;
    if (word == "ON")       return TokenType::ON;
    if (word == "OF")       return TokenType::OF;
    if (word == "TYPE")     return TokenType::TYPE;
    if (word == "ORDER")    return TokenType::ORDER;
    if (word == "BY")       return TokenType::BY;
    if (word == "ASC")      return TokenType::ASC;
    if (word == "DESC")     return TokenType::DESC;
    if (word == "BTREE")    return TokenType::BTREE;
    if (word == "BST")      return TokenType::BST;
    if (word == "INTEGER")  return TokenType::INTEGER;
    if (word == "DOUBLE")   return TokenType::DOUBLE;
    if (word == "VARCHAR")  return TokenType::VARCHAR;
    if (word == "DATETIME") return TokenType::DATETIME;
    if (word == "LIKE")     return TokenType::LIKE;
    if (word == "NOT")      return TokenType::NOT;
    return TokenType::UNKNOWN;
}