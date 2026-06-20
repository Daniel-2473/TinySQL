// query/SemanticValidator.cpp
#include "SemanticValidator.h"
#include "../../SystemCatalog.h"  

void SemanticValidator::setError(ParsedQuery& q, const std::string& msg) {
    q.hasError     = true;
    q.errorMessage = msg;
}

bool SemanticValidator::requireDatabase(ParsedQuery& q, const std::string& db) {
    if (db.empty()) {
        setError(q, "No hay base de datos seleccionada. Use SET DATABASE primero.");
        return false;
    }
    return true;
}

void SemanticValidator::validate(ParsedQuery& query, const std::string& currentDatabase) {
    // Si ya tiene error del parser, no validamos
    if (query.hasError) return;

    switch (query.type) {
        case QueryType::CREATE_DATABASE:
            validateCreateDatabase(query);
            break;
        case QueryType::SET_DATABASE:
            validateSetDatabase(query);
            break;
        case QueryType::CREATE_TABLE:
            validateCreateTable(query, currentDatabase);
            break;
        case QueryType::DROP_TABLE:
            validateDropTable(query, currentDatabase);
            break;
        case QueryType::CREATE_INDEX:
            validateCreateIndex(query, currentDatabase);
            break;
        case QueryType::SELECT:
            validateSelect(query, currentDatabase);
            break;
        case QueryType::INSERT:
            validateInsert(query, currentDatabase);
            break;
        case QueryType::UPDATE:
            validateUpdate(query, currentDatabase);
            break;
        case QueryType::DELETE:
            validateDelete(query, currentDatabase);
            break;
        default:
            setError(query, "Tipo de consulta desconocido.");
    }
}

void SemanticValidator::validateCreateDatabase(ParsedQuery& q) {
    // No puede existir ya
    if (databaseExists(q.databaseName)) {
        setError(q, "La base de datos '" + q.databaseName + "' ya existe.");
    }
}

void SemanticValidator::validateSetDatabase(ParsedQuery& q) {
    // Debe existir
    if (!databaseExists(q.databaseName)) {
        setError(q, "La base de datos '" + q.databaseName + "' no existe.");
    }
}

void SemanticValidator::validateCreateTable(ParsedQuery& q, const std::string& db) {
    if (!requireDatabase(q, db)) return;

    // No puede existir ya
    if (tableExists(db, q.tableName)) {
        setError(q, "La tabla '" + q.tableName + "' ya existe en '" + db + "'.");
        return;
    }

    // Debe tener al menos una columna
    if (q.tableColumns.empty()) {
        setError(q, "La tabla debe tener al menos una columna.");
        return;
    }

    // Validar que no haya columnas duplicadas
    for (int i = 0; i < q.tableColumns.size(); i++) {
        for (int j = i + 1; j < q.tableColumns.size(); j++) {
            if (q.tableColumns[i].name == q.tableColumns[j].name) {
                setError(q, "Columna duplicada: '" + q.tableColumns[i].name + "'.");
                return;
            }
        }
    }
}

void SemanticValidator::validateDropTable(ParsedQuery& q, const std::string& db) {
    if (!requireDatabase(q, db)) return;

    if (!tableExists(db, q.tableName)) {
        setError(q, "La tabla '" + q.tableName + "' no existe.");
    }
    // Nota: la verificación de que esté vacía la hace el StoredDataManager
}

void SemanticValidator::validateCreateIndex(ParsedQuery& q, const std::string& db) {
    if (!requireDatabase(q, db)) return;

    if (!tableExists(db, q.tableName)) {
        setError(q, "La tabla '" + q.tableName + "' no existe.");
        return;
    }

    if (!columnExists(db, q.tableName, q.indexColumn)) {
        setError(q, "La columna '" + q.indexColumn + "' no existe en '" + q.tableName + "'.");
        return;
    }

    // Solo puede haber un índice por columna
    if (indexExists(db, q.tableName, q.indexColumn)) {
        setError(q, "Ya existe un índice en la columna '" + q.indexColumn + "'.");
    }
}

void SemanticValidator::validateSelect(ParsedQuery& q, const std::string& db) {
    if (!requireDatabase(q, db)) return;

    if (!tableExists(db, q.tableName)) {
        setError(q, "La tabla '" + q.tableName + "' no existe.");
        return;
    }

    // Validar columnas del SELECT (si no es *)
    if (!(q.selectColumns.size() == 1 && q.selectColumns[0] == "*")) {
        for (const auto& col : q.selectColumns) {
            if (!columnExists(db, q.tableName, col)) {
                setError(q, "La columna '" + col + "' no existe en '" + q.tableName + "'.");
                return;
            }
        }
    }

    // Validar columna del WHERE
    if (q.hasWhere && !columnExists(db, q.tableName, q.where.column)) {
        setError(q, "La columna '" + q.where.column + "' no existe en '" + q.tableName + "'.");
        return;
    }

    // Validar columna del ORDER BY
    if (q.hasOrderBy && !columnExists(db, q.tableName, q.orderByColumn)) {
        setError(q, "La columna '" + q.orderByColumn + "' no existe en '" + q.tableName + "'.");
    }
}

void SemanticValidator::validateInsert(ParsedQuery& q, const std::string& db) {
    if (!requireDatabase(q, db)) return;

    if (!tableExists(db, q.tableName)) {
        setError(q, "La tabla '" + q.tableName + "' no existe.");
        return;
    }

    // Verificar que el número de valores coincida con el número de columnas
    int colCount = countColumns(db, q.tableName);
    if (q.insertValues.size() != colCount) {
        setError(q, "Se esperaban " + std::to_string(colCount) +
                    " valores pero se recibieron " +
                    std::to_string(q.insertValues.size()) + ".");
    }
}

void SemanticValidator::validateUpdate(ParsedQuery& q, const std::string& db) {
    if (!requireDatabase(q, db)) return;

    if (!tableExists(db, q.tableName)) {
        setError(q, "La tabla '" + q.tableName + "' no existe.");
        return;
    }

    // Validar columnas del SET
    for (const auto& pair : q.setValues) {
        if (!columnExists(db, q.tableName, pair.first)) {
            setError(q, "La columna '" + pair.first + "' no existe en '" + q.tableName + "'.");
            return;
        }
    }

    // Validar columna del WHERE
    if (q.hasWhere && !columnExists(db, q.tableName, q.where.column)) {
        setError(q, "La columna '" + q.where.column + "' no existe en '" + q.tableName + "'.");
    }
}

void SemanticValidator::validateDelete(ParsedQuery& q, const std::string& db) {
    if (!requireDatabase(q, db)) return;

    if (!tableExists(db, q.tableName)) {
        setError(q, "La tabla '" + q.tableName + "' no existe.");
        return;
    }

    // Validar columna del WHERE
    if (q.hasWhere && !columnExists(db, q.tableName, q.where.column)) {
        setError(q, "La columna '" + q.where.column + "' no existe en '" + q.tableName + "'.");
    }
}