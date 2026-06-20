#pragma once
#include <string>

enum class TokenType {
    // Palabras clave
    SELECT, FROM, WHERE, INSERT, INTO, VALUES,
    UPDATE, SET, DELETE, CREATE, DROP, DATABASE,
    TABLE, INDEX, ON, OF, TYPE, ORDER, BY, ASC, DESC,
    BTREE, BST,
    // Tipos de datos
    INTEGER, DOUBLE, VARCHAR, DATETIME,
    // Símbolos
    STAR, COMMA, SEMICOLON, LPAREN, RPAREN,
    EQUALS, GREATER, LESS, LIKE, NOT,
    // Valores
    IDENTIFIER, STRING_LITERAL, NUMBER_LITERAL,
    // Control
    END_OF_INPUT, UNKNOWN
};

struct Token {
    TokenType   type;
    std::string value;
};