#include "Parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), pos(0) {}

Token& Parser::current() {
    return tokens[pos];
}

Token& Parser::peek() {
    if (pos + 1 < tokens.size()) return tokens[pos + 1];
    return tokens[pos];
}

void Parser::advance() {
    if (pos < tokens.size() - 1) pos++;
}

Token Parser::expect(TokenType type, const std::string& errorMsg) {
    if (current().type != type) {
        throw std::runtime_error(errorMsg + " (encontrado: '" + current().value + "')");
    }
    Token t = current();
    advance();
    return t;
}

ParsedQuery Parser::makeError(const std::string& msg) {
    ParsedQuery q;
    q.hasError     = true;
    q.errorMessage = msg;
    return q;
}

ParsedQuery Parser::parseCreate() {
    advance(); // consume CREATE

    if (current().type == TokenType::DATABASE) {
        advance(); // consume DATABASE
        ParsedQuery q;
        q.type         = QueryType::CREATE_DATABASE;
        q.databaseName = expect(TokenType::IDENTIFIER,
                          "Se esperaba el nombre de la base de datos").value;
        return q;
    }

    if (current().type == TokenType::TABLE) {
        advance(); // consume TABLE
        ParsedQuery q;
        q.type      = QueryType::CREATE_TABLE;
        q.tableName = expect(TokenType::IDENTIFIER,
                      "Se esperaba el nombre de la tabla").value;

        expect(TokenType::LPAREN, "Se esperaba '(' después del nombre de la tabla");

        // Leer columnas hasta encontrar ')'
        while (current().type != TokenType::RPAREN &&
               current().type != TokenType::END_OF_INPUT) {
            q.tableColumns.push_back(parseColumnDefinition());

            if (current().type == TokenType::COMMA)
                advance(); // consume la coma entre columnas
        }

        expect(TokenType::RPAREN, "Se esperaba ')' al cerrar la definición de tabla");
        return q;
    }

    if (current().type == TokenType::INDEX) {
        advance(); // consume INDEX
        ParsedQuery q;
        q.type        = QueryType::CREATE_INDEX;
        q.indexName   = expect(TokenType::IDENTIFIER,
                        "Se esperaba el nombre del índice").value;

        expect(TokenType::ON, "Se esperaba ON");
        q.tableName   = expect(TokenType::IDENTIFIER,
                        "Se esperaba el nombre de la tabla").value;

        expect(TokenType::LPAREN, "Se esperaba '('");
        q.indexColumn = expect(TokenType::IDENTIFIER,
                        "Se esperaba el nombre de la columna").value;
        expect(TokenType::RPAREN, "Se esperaba ')'");

        expect(TokenType::OF,   "Se esperaba OF");
        expect(TokenType::TYPE, "Se esperaba TYPE");

        if (current().type == TokenType::BTREE) {
            q.indexType = "BTREE";
        } else if (current().type == TokenType::BST) {
            q.indexType = "BST";
        } else {
            throw std::runtime_error("Tipo de índice inválido, use BTREE o BST");
        }
        advance();
        return q;
    }

    return makeError("Se esperaba DATABASE, TABLE o INDEX después de CREATE");
}

ParsedQuery Parser::parseSet() {
    advance(); // consume SET

    // SET puede ser SET DATABASE o UPDATE SET
    // Aquí solo manejamos SET DATABASE
    if (current().type == TokenType::DATABASE) {
        advance(); // consume DATABASE
        ParsedQuery q;
        q.type         = QueryType::SET_DATABASE;
        q.databaseName = expect(TokenType::IDENTIFIER,
                          "Se esperaba el nombre de la base de datos").value;
        return q;
    }

    return makeError("Se esperaba DATABASE después de SET");
}

ParsedQuery Parser::parseDrop() {
    advance(); // consume DROP
    expect(TokenType::TABLE, "Se esperaba TABLE después de DROP");

    ParsedQuery q;
    q.type      = QueryType::DROP_TABLE;
    q.tableName = expect(TokenType::IDENTIFIER,
                  "Se esperaba el nombre de la tabla").value;
    return q;
}

ParsedQuery Parser::parseSelect() {
    advance(); // consume SELECT
    ParsedQuery q;
    q.type = QueryType::SELECT;

    // Leer columnas: * o lista separada por comas
    if (current().type == TokenType::STAR) {
        q.selectColumns.push_back("*");
        advance();
    } else {
        q.selectColumns.push_back(
            expect(TokenType::IDENTIFIER, "Se esperaba columna").value);

        while (current().type == TokenType::COMMA) {
            advance(); // consume la coma
            q.selectColumns.push_back(
                expect(TokenType::IDENTIFIER, "Se esperaba columna").value);
        }
    }

    expect(TokenType::FROM, "Se esperaba FROM");
    q.tableName = expect(TokenType::IDENTIFIER,
                  "Se esperaba nombre de tabla").value;

    // WHERE opcional
    if (current().type == TokenType::WHERE) {
        advance();
        q.where    = parseWhere();
        q.hasWhere = true;
    }

    // ORDER BY opcional
    if (current().type == TokenType::ORDER) {
        advance();
        expect(TokenType::BY, "Se esperaba BY después de ORDER");
        q.orderByColumn    = expect(TokenType::IDENTIFIER,
                             "Se esperaba columna para ORDER BY").value;
        q.hasOrderBy       = true;
        q.orderByDirection = "ASC"; // por defecto

        if (current().type == TokenType::ASC) {
            q.orderByDirection = "ASC";
            advance();
        } else if (current().type == TokenType::DESC) {
            q.orderByDirection = "DESC";
            advance();
        }
    }

    return q;
}

ParsedQuery Parser::parseInsert() {
    advance(); // consume INSERT
    expect(TokenType::INTO, "Se esperaba INTO");

    ParsedQuery q;
    q.type      = QueryType::INSERT;
    q.tableName = expect(TokenType::IDENTIFIER,
                  "Se esperaba nombre de tabla").value;

    expect(TokenType::VALUES, "Se esperaba VALUES");
    expect(TokenType::LPAREN, "Se esperaba '('");

    // Leer valores separados por comas
    while (current().type != TokenType::RPAREN &&
           current().type != TokenType::END_OF_INPUT) {

        if (current().type == TokenType::STRING_LITERAL ||
            current().type == TokenType::NUMBER_LITERAL ||
            current().type == TokenType::IDENTIFIER) {
            q.insertValues.push_back(current().value);
            advance();
        } else {
            throw std::runtime_error("Valor inválido en INSERT: '" +
                                     current().value + "'");
        }

        if (current().type == TokenType::COMMA)
            advance();
    }

    expect(TokenType::RPAREN, "Se esperaba ')'");
    return q;
}

ParsedQuery Parser::parseUpdate() {
    advance(); // consume UPDATE
    ParsedQuery q;
    q.type      = QueryType::UPDATE;
    q.tableName = expect(TokenType::IDENTIFIER,
                  "Se esperaba nombre de tabla").value;

    expect(TokenType::SET, "Se esperaba SET");

    // Leer pares columna = valor
    do {
        std::string col = expect(TokenType::IDENTIFIER,
                          "Se esperaba nombre de columna").value;
        expect(TokenType::EQUALS, "Se esperaba '='");

        std::string val;
        if (current().type == TokenType::STRING_LITERAL ||
            current().type == TokenType::NUMBER_LITERAL) {
            val = current().value;
            advance();
        } else {
            throw std::runtime_error("Valor inválido en SET");
        }

        q.setValues.push_back({col, val});

        if (current().type == TokenType::COMMA)
            advance();

    } while (current().type == TokenType::IDENTIFIER);

    // WHERE opcional
    if (current().type == TokenType::WHERE) {
        advance();
        q.where    = parseWhere();
        q.hasWhere = true;
    }

    return q;
}

WhereClause Parser::parseWhere() {
    WhereClause w;
    w.column = expect(TokenType::IDENTIFIER,
               "Se esperaba nombre de columna en WHERE").value;

    // Operador
    if (current().type == TokenType::EQUALS) {
        w.op = "=";
    } else if (current().type == TokenType::GREATER) {
        w.op = ">";
    } else if (current().type == TokenType::LESS) {
        w.op = "<";
    } else if (current().type == TokenType::LIKE) {
        w.op = "LIKE";
    } else if (current().type == TokenType::NOT) {
        w.op = "NOT";
    } else {
        throw std::runtime_error("Operador inválido en WHERE: '" +
                                 current().value + "'");
    }
    advance();

    // Valor
    if (current().type == TokenType::STRING_LITERAL ||
        current().type == TokenType::NUMBER_LITERAL ||
        current().type == TokenType::IDENTIFIER) {
        w.value = current().value;
        advance();
    } else {
        throw std::runtime_error("Valor inválido en WHERE");
    }

    return w;
}

ColumnDefinition Parser::parseColumnDefinition() {
    ColumnDefinition col;
    col.name = expect(TokenType::IDENTIFIER,
               "Se esperaba nombre de columna").value;

    // Tipo de dato
    if (current().type == TokenType::INTEGER) {
        col.type = "INTEGER";
        advance();
    } else if (current().type == TokenType::DOUBLE) {
        col.type = "DOUBLE";
        advance();
    } else if (current().type == TokenType::DATETIME) {
        col.type = "DATETIME";
        advance();
    } else if (current().type == TokenType::VARCHAR) {
        col.type = "VARCHAR";
        advance();
        expect(TokenType::LPAREN, "Se esperaba '(' después de VARCHAR");
        col.varcharLength = std::stoi(
            expect(TokenType::NUMBER_LITERAL,
                   "Se esperaba el largo del VARCHAR").value);
        expect(TokenType::RPAREN, "Se esperaba ')'");
    } else {
        throw std::runtime_error("Tipo de dato inválido: '" +
                                 current().value + "'");
    }

    return col;
}

ParsedQuery Parser::parseDelete() {
    advance(); // consume DELETE
    expect(TokenType::FROM, "Se esperaba FROM");

    ParsedQuery q;
    q.type      = QueryType::DELETE;
    q.tableName = expect(TokenType::IDENTIFIER,
                  "Se esperaba nombre de tabla").value;

    if (current().type == TokenType::WHERE) {
        advance();
        q.where    = parseWhere();
        q.hasWhere = true;
    }

    return q;
}

ParsedQuery Parser::parse() {
    try {
        switch (current().type) {
            case TokenType::SELECT:   return parseSelect();
            case TokenType::INSERT:   return parseInsert();
            case TokenType::UPDATE:   return parseUpdate();
            case TokenType::DELETE:   return parseDelete();
            case TokenType::CREATE:   return parseCreate();
            case TokenType::DROP:     return parseDrop();
            case TokenType::SET:      return parseSet();
            default:
                return makeError("Comando SQL no reconocido: '" + current().value + "'");
        }
    } catch (const std::runtime_error& e) {
        return makeError(e.what());
    }
}