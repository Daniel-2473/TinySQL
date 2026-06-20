#pragma once
#include <string>
#include <vector>
#include "query/ParsedQuery.h"   
#include "QueryResult.h"         
#include "SystemCatalog.h"       
#include "Quicksort.h"           

class StoredDataManager {
public:
    // DDL
    QueryResult execCreateDatabase(const ParsedQuery& q);
    QueryResult execCreateTable   (const ParsedQuery& q, const std::string& db);
    QueryResult execDropTable     (const ParsedQuery& q, const std::string& db);
    QueryResult execCreateIndex   (const ParsedQuery& q, const std::string& db);

    // DML
    QueryResult execInsert   (const ParsedQuery& q, const std::string& db);
    QueryResult execSelect   (const ParsedQuery& q, const std::string& db);
    QueryResult execUpdate   (const ParsedQuery& q, const std::string& db);
    QueryResult execDelete   (const ParsedQuery& q, const std::string& db);

private:
    // ── helpers de lectura ──────────────────────────────────────────────────

    // Lee todas las columnas de una tabla ordenadas por posición
    std::vector<ColumnRecord> getColumns(const std::string& db,
                                         const std::string& table);

    // Lee TODAS las filas del archivo .bin y las devuelve como Row (desencriptadas)
    std::vector<Row> readAllRows(const std::string& db,
                                 const std::string& table,
                                 const std::vector<ColumnRecord>& cols,
                                 int rowSize);

    // Convierte el buffer binario desencriptado de una fila a vector<string>
    std::vector<std::string> bufferToStrings(const char* buf,
                                              const std::vector<ColumnRecord>& cols);

    // ── helpers de evaluación WHERE ─────────────────────────────────────────

    // Devuelve true si la fila cumple la condición WHERE
    bool matchesWhere(const Row& row,
                      const WhereClause& where,
                      int colIndex,
                      const std::string& dataType);

    // ── helpers de índices ──────────────────────────────────────────────────

    // Busca la posición en disco de una fila usando el índice (BST o BTree)
    // Devuelve -1 si no hay índice o no se encuentra
    long searchIndex(const std::string& mapKey, int intVal);

    // Elimina la entrada del índice en memoria
    void removeFromIndex(const std::string& mapKey, int intVal);

    // Actualiza la entrada del índice (remove + insert)
    void updateIndex(const std::string& mapKey, int oldVal, int newVal, long newPos);
};
