#pragma once
#include <string>
#include <vector>

enum class QueryType {
    CREATE_DATABASE,
    SET_DATABASE,
    CREATE_TABLE,
    DROP_TABLE,
    CREATE_INDEX,
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    UNKNOWN
};

struct ColumnDefinition {
    std::string name;
    std::string type;      // INTEGER, DOUBLE, VARCHAR, DATETIME
    int         varcharLength = 0;
    bool        nullable  = true;
};

struct WhereClause {
    std::string column;
    std::string op;        // =, >, <, LIKE, NOT
    std::string value;
};

struct ParsedQuery {
    QueryType   type = QueryType::UNKNOWN;
    std::string databaseName;
    std::string tableName;
    std::string indexName;
    std::string indexType;      // BTREE o BST
    std::string indexColumn;
    std::string orderByColumn;
    std::string orderByDirection; // ASC o DESC

    std::vector<std::string>      selectColumns; // * o columnas específicas
    std::vector<ColumnDefinition> tableColumns;  // para CREATE TABLE
    std::vector<std::string>      insertValues;  // para INSERT
    std::vector<std::pair<std::string, std::string>> setValues; // para UPDATE

    WhereClause where;
    bool        hasWhere     = false;
    bool        hasOrderBy   = false;

    // Si hubo error
    bool        hasError     = false;
    std::string errorMessage;
};