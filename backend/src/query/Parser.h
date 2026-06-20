// query/Parser.h
#pragma once
#include "Token.h"
#include "ParsedQuery.h"
#include <vector>

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    ParsedQuery parse();

private:
    std::vector<Token> tokens;
    int                pos = 0;

    // Navegación por los tokens
    Token& current();
    Token& peek();
    void   advance();

    // Consume el token actual si es del tipo esperado
    // Si no, marca error
    Token  expect(TokenType type, const std::string& errorMsg);

    // Parsers de cada comando
    ParsedQuery parseSelect();
    ParsedQuery parseInsert();
    ParsedQuery parseUpdate();
    ParsedQuery parseDelete();
    ParsedQuery parseCreate();
    ParsedQuery parseDrop();
    ParsedQuery parseSet();

    // Parsers de sub-estructuras
    WhereClause      parseWhere();
    ColumnDefinition parseColumnDefinition();

    // Helpers
    ParsedQuery makeError(const std::string& msg);
};