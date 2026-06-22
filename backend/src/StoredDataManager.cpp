// StoredDataManager.cpp
// Capa de acceso a datos: lee/escribe archivos .bin encriptados con XOR,
// usa BST / BTree en memoria para búsquedas indexadas y Quicksort para ORDER BY.

#include "StoredDataManager.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <iostream>

using namespace std;

// ═══════════════════════════════════════════════════════════════════════════════
// HELPERS PRIVADOS
// ═══════════════════════════════════════════════════════════════════════════════

vector<ColumnRecord> StoredDataManager::getColumns(const string& db, const string& table) {
    fstream file("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);
    vector<ColumnRecord> cols;
    ColumnRecord cr;
    while (file.read(reinterpret_cast<char*>(&cr), sizeof(ColumnRecord))) {
        if (string(cr.databaseName) == db && string(cr.tableName) == table)
            cols.push_back(cr);
    }
    file.close();
    sort(cols.begin(), cols.end(), [](const ColumnRecord& a, const ColumnRecord& b) {
        return a.position < b.position;
    });
    return cols;
}

// Convierte un buffer binario desencriptado a strings legibles
vector<string> StoredDataManager::bufferToStrings(const char* buf,
                                                    const vector<ColumnRecord>& cols) {
    vector<string> result;
    int offset = 0;
    for (const auto& col : cols) {
        string type = string(col.dataType);
        if (type == "INTEGER") {
            int v; memcpy(&v, buf + offset, sizeof(int));
            result.push_back(to_string(v));
            offset += sizeof(int);
        } else if (type == "DOUBLE") {
            double v; memcpy(&v, buf + offset, sizeof(double));
            result.push_back(to_string(v));
            offset += sizeof(double);
        } else if (type == "VARCHAR") {
            char tmp[col.maxSize + 1];
            memset(tmp, 0, col.maxSize + 1);
            memcpy(tmp, buf + offset, col.maxSize);
            string s(tmp);
            // Quitar caracteres nulos y espacios del final
            size_t end = s.find_last_not_of(string("\0 ", 2));
            if (end != string::npos)
                s = s.substr(0, end + 1);
            else
                s = "";
            result.push_back(s);
            offset += col.maxSize;
        } else if (type == "DATETIME") {
            DateTime dt; memcpy(&dt, buf + offset, sizeof(DateTime));
            char tmp[30];
            snprintf(tmp, sizeof(tmp), "%04d-%02d-%02d %02d:%02d:%02d",
                     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
            result.push_back(string(tmp));
            offset += sizeof(DateTime);
        }
    }
    return result;
}

// Lee todas las filas del .bin y las devuelve desencriptadas como Row
vector<Row> StoredDataManager::readAllRows(const string& db, const string& table,
                                            const vector<ColumnRecord>& cols, int rowSize) {
    vector<Row> rows;
    string path = db + "/" + table + ".bin";
    fstream file(path, ios::in | ios::binary);
    if (!file.is_open()) return rows;

    char* buf = new char[rowSize];
    long pos = 0;
    while (file.read(buf, rowSize)) {
        xorEncryptDecrypt(buf, rowSize);          // desencriptar
        Row row;
        row.values   = bufferToStrings(buf, cols);
        row.position = pos;
        rows.push_back(row);
        pos += rowSize;
    }
    delete[] buf;
    file.close();
    return rows;
}

// Evalúa si una fila cumple la condición WHERE
bool StoredDataManager::matchesWhere(const Row& row, const WhereClause& where,
                                      int colIndex, const string& dataType) {
    const string& cell  = row.values[colIndex];
    const string& val   = where.value;
    const string& op    = where.op;   // >, <, =, like, not

    std::cout << "DEBUG matchesWhere:"
              << " cell='" << cell << "'"
              << " op='"   << op   << "'"
              << " val='"  << val  << "'"
              << " dataType='" << dataType << "'" << std::endl;

    if (op == "=") {
        if (dataType == "INTEGER") return stoi(cell) == stoi(val);
        if (dataType == "DOUBLE")  return stod(cell) == stod(val);
        return cell == val;
    }
    if (op == ">") {
        if (dataType == "INTEGER") return stoi(cell) > stoi(val);
        if (dataType == "DOUBLE")  return stod(cell) > stod(val);
        return cell > val;
    }
    if (op == "<") {
        if (dataType == "INTEGER") return stoi(cell) < stoi(val);
        if (dataType == "DOUBLE")  return stod(cell) < stod(val);
        return cell < val;
    }
    if (op == "LIKE") {
        string pattern = val;
        bool startWild = (!pattern.empty() && pattern.front() == '*');
        bool endWild   = (!pattern.empty() && pattern.back()  == '*');
        if (startWild) pattern = pattern.substr(1);
        if (endWild)   pattern = pattern.substr(0, pattern.size() - 1);

        if (startWild && endWild) return cell.find(pattern) != string::npos;
        if (startWild)            return cell.size() >= pattern.size() &&
                                        cell.substr(cell.size() - pattern.size()) == pattern;
        if (endWild)              return cell.substr(0, pattern.size()) == pattern;

        // Sin asteriscos: buscar si el patrón está contenido en cualquier parte
        return cell.find(pattern) != string::npos;
    }
    if (op == "NOT") {
        if (dataType == "INTEGER") return stoi(cell) != stoi(val);
        if (dataType == "DOUBLE")  return stod(cell) != stod(val);
        return cell != val;
    }
    return false;
}

// Busca en el índice en memoria; devuelve posición en disco o -1
long StoredDataManager::searchIndex(const string& mapKey, int intVal) {
    if (bstIntIndexes.count(mapKey))   return bstIntIndexes[mapKey]->search(intVal);
    if (btreeIntIndexes.count(mapKey)) return btreeIntIndexes[mapKey]->search(intVal);
    return -1;
}

void StoredDataManager::removeFromIndex(const string& mapKey, int intVal) {
    if (bstIntIndexes.count(mapKey))   bstIntIndexes[mapKey]->remove(intVal);
    if (btreeIntIndexes.count(mapKey)) btreeIntIndexes[mapKey]->remove(intVal);
}

void StoredDataManager::updateIndex(const string& mapKey, int oldVal, int newVal, long newPos) {
    removeFromIndex(mapKey, oldVal);
    if (bstIntIndexes.count(mapKey))   bstIntIndexes[mapKey]->insert(newVal, newPos);
    if (btreeIntIndexes.count(mapKey)) btreeIntIndexes[mapKey]->insert(newVal, newPos);
}

// ═══════════════════════════════════════════════════════════════════════════════
// DDL
// ═══════════════════════════════════════════════════════════════════════════════

QueryResult StoredDataManager::execCreateDatabase(const ParsedQuery& q) {
    QueryResult result;
    // La validación semántica ya verificó que no existe.
    // SystemCatalog::createDatabase crea la carpeta y registra el record.
    ::createDatabase(q.databaseName);
    result.success = true;
    result.message = "Base de datos '" + q.databaseName + "' creada exitosamente.";
    return result;
}

QueryResult StoredDataManager::execCreateTable(const ParsedQuery& q, const string& db) {
    QueryResult result;
    // 1. Crear la entrada en SystemTables y el archivo .bin vacío
    ::createTable(db, q.tableName, (int)q.tableColumns.size());

    // 2. Registrar cada columna en SystemColumns
    for (int i = 0; i < (int)q.tableColumns.size(); i++) {
        const auto& col = q.tableColumns[i];
        ::addColumn(db, q.tableName, col.name, col.type, col.varcharLength, i);
    }

    result.success = true;
    result.message = "Tabla '" + q.tableName + "' creada exitosamente.";
    return result;
}

QueryResult StoredDataManager::execDropTable(const ParsedQuery& q, const string& db) {
    QueryResult result;
    // SystemCatalog::dropTable ya verifica que esté vacía y limpia todo
    ::dropTable(db, q.tableName);
    result.success = true;
    result.message = "Tabla '" + q.tableName + "' eliminada exitosamente.";
    return result;
}

QueryResult StoredDataManager::execCreateIndex(const ParsedQuery& q, const string& db) {
    QueryResult result;
    // SystemCatalog::createIndex construye el árbol en memoria y registra el índice
    ::createIndex(db, q.tableName, q.indexColumn, q.indexName, q.indexType);
    result.success = true;
    result.message = "Índice '" + q.indexName + "' creado sobre '" +
                     q.tableName + "(" + q.indexColumn + ")'.";
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// INSERT
// ═══════════════════════════════════════════════════════════════════════════════

QueryResult StoredDataManager::execInsert(const ParsedQuery& q, const string& db) {
    QueryResult result;

    // Construir el vector<RowValue> que espera insertRow del SystemCatalog.
    // Los valores llegan en q.insertValues en el mismo orden que las columnas de la tabla.
    vector<ColumnRecord> cols = getColumns(db, q.tableName);

    if (q.insertValues.size() != cols.size()) {
        result.success = false;
        result.message = "Se esperaban " + to_string(cols.size()) +
                         " valores, se recibieron " + to_string(q.insertValues.size()) + ".";
        return result;
    }

    vector<RowValue> rowValues;
    for (int i = 0; i < (int)cols.size(); i++) {
        RowValue rv;
        rv.columnName = string(cols[i].columnName);
        rv.value      = q.insertValues[i];
        rowValues.push_back(rv);
    }

    // insertRow maneja encriptación, verificación de duplicados en índices y actualización de árboles
    ::insertRow(db, q.tableName, rowValues);

    result.success = true;
    result.message = "Fila insertada correctamente en '" + q.tableName + "'.";
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// SELECT
// ═══════════════════════════════════════════════════════════════════════════════

QueryResult StoredDataManager::execSelect(const ParsedQuery& q, const string& db) {
    QueryResult result;

    int rowSize = calculateRowSize(db, q.tableName);
    if (rowSize <= 0) {
        result.success = false;
        result.message = "No se pudo calcular el tamaño de fila.";
        return result;
    }

    vector<ColumnRecord> cols = getColumns(db, q.tableName);

    // ── Determinar columnas a proyectar ───────────────────────────────────────
    bool selectAll = (q.selectColumns.size() == 1 && q.selectColumns[0] == "*");
    vector<int> projIndexes; // índices de columnas a incluir en el resultado
    if (selectAll) {
        for (int i = 0; i < (int)cols.size(); i++) projIndexes.push_back(i);
    } else {
        for (const auto& colName : q.selectColumns) {
            for (int i = 0; i < (int)cols.size(); i++) {
                if (string(cols[i].columnName) == colName) {
                    projIndexes.push_back(i);
                    break;
                }
            }
        }
    }

    // ── Determinar columna e índice del WHERE ─────────────────────────────────
    int whereColIndex = -1;
    string whereDataType;
    string whereMapKey;
    if (q.hasWhere) {
        for (int i = 0; i < (int)cols.size(); i++) {
            if (string(cols[i].columnName) == q.where.column) {
                whereColIndex  = i;
                whereDataType  = string(cols[i].dataType);
                whereMapKey    = db + "." + q.tableName + "." + q.where.column;
                break;
            }
        }
    }

    // ── Intentar búsqueda indexada (solo INTEGER con op "=") ──────────────────
    vector<Row> matchedRows;
    bool usedIndex = false;

    if (q.hasWhere && whereDataType == "INTEGER" && q.where.op == "=") {
        long diskPos = searchIndex(whereMapKey, stoi(q.where.value));
        if (diskPos >= 0) {
            usedIndex = true;
            string path = db + "/" + q.tableName + ".bin";
            fstream file(path, ios::in | ios::binary);
            file.seekg(diskPos);
            char* buf = new char[rowSize];
            if (file.read(buf, rowSize)) {
                xorEncryptDecrypt(buf, rowSize);
                Row row;
                row.values   = bufferToStrings(buf, cols);
                row.position = diskPos;
                matchedRows.push_back(row);
            }
            delete[] buf;
            file.close();
        }
    }

    // ── Búsqueda secuencial si no se usó índice ───────────────────────────────
    if (!usedIndex) {
        vector<Row> allRows = readAllRows(db, q.tableName, cols, rowSize);
        if (!q.hasWhere) {
            matchedRows = allRows;
        } else {
            for (const auto& row : allRows) {
                if (matchesWhere(row, q.where, whereColIndex, whereDataType))
                    matchedRows.push_back(row);
            }
        }
    }

    // ── ORDER BY con Quicksort ────────────────────────────────────────────────
    if (q.hasOrderBy && !matchedRows.empty()) {
        int orderColIndex = -1;
        string orderDataType;
        for (int i = 0; i < (int)cols.size(); i++) {
            if (string(cols[i].columnName) == q.orderByColumn) {
                orderColIndex = i;
                orderDataType = string(cols[i].dataType);
                break;
            }
        }
        if (orderColIndex >= 0) {
            bool ascending = (q.orderByDirection == "ASC");
            sortRows(matchedRows, orderColIndex, orderDataType, ascending);
        }
    }

    // ── Construir resultado ───────────────────────────────────────────────────
    for (int idx : projIndexes)
        result.columns.push_back(string(cols[idx].columnName));

    for (const auto& row : matchedRows) {
        vector<string> projRow;
        for (int idx : projIndexes)
            projRow.push_back(row.values[idx]);
        result.rows.push_back(projRow);
    }

    result.success = true;
    result.message = to_string(result.rows.size()) + " fila(s) encontrada(s).";
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// UPDATE
// ═══════════════════════════════════════════════════════════════════════════════

QueryResult StoredDataManager::execUpdate(const ParsedQuery& q, const string& db) {
    QueryResult result;

    int rowSize = calculateRowSize(db, q.tableName);
    if (rowSize <= 0) {
        result.success = false;
        result.message = "No se pudo calcular el tamaño de fila.";
        return result;
    }

    vector<ColumnRecord> cols = getColumns(db, q.tableName);

    // ── Índice del WHERE ──────────────────────────────────────────────────────
    int whereColIndex = -1;
    string whereDataType, whereMapKey;
    if (q.hasWhere) {
        for (int i = 0; i < (int)cols.size(); i++) {
            if (string(cols[i].columnName) == q.where.column) {
                whereColIndex = i;
                whereDataType = string(cols[i].dataType);
                whereMapKey   = db + "." + q.tableName + "." + q.where.column;
                break;
            }
        }
    }

    // ── Calcular offsets de las columnas SET ──────────────────────────────────
    // Mapa: nombre_columna -> (offset en bytes, indice en cols, dataType)
    struct ColInfo { int offset; int colIdx; string dataType; };
    map<string, ColInfo> colInfoMap;
    {
        int off = 0;
        for (int i = 0; i < (int)cols.size(); i++) {
            string t = string(cols[i].dataType);
            colInfoMap[string(cols[i].columnName)] = {off, i, t};
            if      (t == "INTEGER")  off += sizeof(int);
            else if (t == "DOUBLE")   off += sizeof(double);
            else if (t == "VARCHAR")  off += cols[i].maxSize;
            else if (t == "DATETIME") off += sizeof(DateTime);
        }
    }

    // ── Abrir el archivo .bin en modo lectura/escritura ───────────────────────
    string path = db + "/" + q.tableName + ".bin";
    fstream file(path, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        result.success = false;
        result.message = "No se pudo abrir la tabla.";
        return result;
    }

    // ── Leer todas las filas y modificar las que coincidan ───────────────────
    int updatedCount = 0;
    char* buf = new char[rowSize];
    long pos = 0;

    while (file.read(buf, rowSize)) {
        xorEncryptDecrypt(buf, rowSize);  // desencriptar para leer
        vector<string> vals = bufferToStrings(buf, cols);
        Row row; row.values = vals; row.position = pos;

        bool matches = !q.hasWhere ||
                       matchesWhere(row, q.where, whereColIndex, whereDataType);

        if (matches) {
            // Para cada columna del SET, modificar el buffer
            for (const auto& pair : q.setValues) {
                const string& colName = pair.first;
                const string& newVal  = pair.second;

                if (!colInfoMap.count(colName)) continue;
                auto& ci = colInfoMap[colName];

                // Si la columna tiene índice INTEGER, actualizar el árbol
                if (ci.dataType == "INTEGER") {
                    string mapKey = db + "." + q.tableName + "." + colName;
                    int oldInt; memcpy(&oldInt, buf + ci.offset, sizeof(int));
                    int newInt = stoi(newVal);
                    updateIndex(mapKey, oldInt, newInt, pos);
                    memcpy(buf + ci.offset, &newInt, sizeof(int));
                } else if (ci.dataType == "DOUBLE") {
                    double v = stod(newVal);
                    memcpy(buf + ci.offset, &v, sizeof(double));
                } else if (ci.dataType == "VARCHAR") {
                    memset(buf + ci.offset, 0, cols[ci.colIdx].maxSize);
                    int copyLen = min((int)newVal.size(), cols[ci.colIdx].maxSize);
                    memcpy(buf + ci.offset, newVal.c_str(), copyLen);
                } else if (ci.dataType == "DATETIME") {
                    DateTime dt;
                    sscanf(newVal.c_str(), "%d-%d-%d %d:%d:%d",
                           &dt.year, &dt.month, &dt.day,
                           &dt.hour, &dt.minute, &dt.second);
                    memcpy(buf + ci.offset, &dt, sizeof(DateTime));
                }
            }
            // Re-encriptar y escribir en la misma posición
            xorEncryptDecrypt(buf, rowSize);
            file.seekp(pos);
            file.write(buf, rowSize);
            file.seekg(pos + rowSize);  // restaurar posición de lectura
            updatedCount++;
        }
        pos += rowSize;
    }

    delete[] buf;
    file.close();

    result.success = true;
    result.message = to_string(updatedCount) + " fila(s) actualizada(s).";
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// DELETE
// ═══════════════════════════════════════════════════════════════════════════════

QueryResult StoredDataManager::execDelete(const ParsedQuery& q, const string& db) {
    QueryResult result;

    int rowSize = calculateRowSize(db, q.tableName);
    if (rowSize <= 0) {
        result.success = false;
        result.message = "No se pudo calcular el tamaño de fila.";
        return result;
    }

    vector<ColumnRecord> cols = getColumns(db, q.tableName);

    // ── Índice del WHERE ──────────────────────────────────────────────────────
    int whereColIndex = -1;
    string whereDataType, whereMapKey;
    if (q.hasWhere) {
        for (int i = 0; i < (int)cols.size(); i++) {
            if (string(cols[i].columnName) == q.where.column) {
                whereColIndex = i;
                whereDataType = string(cols[i].dataType);
                whereMapKey   = db + "." + q.tableName + "." + q.where.column;
                break;
            }
        }
    }

    // ── Leer todas las filas, separar las que NO se eliminan ─────────────────
    string path    = db + "/" + q.tableName + ".bin";
    string tmpPath = db + "/" + q.tableName + "_tmp.bin";

    fstream inFile(path, ios::in | ios::binary);
    fstream outFile(tmpPath, ios::out | ios::binary);

    if (!inFile.is_open() || !outFile.is_open()) {
        result.success = false;
        result.message = "Error abriendo archivos de tabla.";
        return result;
    }

    int deletedCount = 0;
    long newPos = 0;   // posición en el archivo temporal (para actualizar índices)
    char* buf = new char[rowSize];
    long pos = 0;

    while (inFile.read(buf, rowSize)) {
        xorEncryptDecrypt(buf, rowSize);
        vector<string> vals = bufferToStrings(buf, cols);
        Row row; row.values = vals; row.position = pos;

        bool shouldDelete = !q.hasWhere ||
                            matchesWhere(row, q.where, whereColIndex, whereDataType);

        if (shouldDelete) {
            // Eliminar del índice en memoria todas las columnas INTEGER indexadas
            for (int i = 0; i < (int)cols.size(); i++) {
                if (string(cols[i].dataType) == "INTEGER") {
                    string mapKey = db + "." + q.tableName + "." + string(cols[i].columnName);
                    if (bstIntIndexes.count(mapKey) || btreeIntIndexes.count(mapKey)) {
                        int v = stoi(vals[i]);
                        removeFromIndex(mapKey, v);
                    }
                }
            }
            deletedCount++;
        } else {
            // Fila que se conserva: re-encriptar y escribir en el temp
            xorEncryptDecrypt(buf, rowSize);
            outFile.write(buf, rowSize);

            // Si la posición en disco cambió, actualizar índices en memoria
            if (newPos != pos) {
                for (int i = 0; i < (int)cols.size(); i++) {
                    if (string(cols[i].dataType) == "INTEGER") {
                        string mapKey = db + "." + q.tableName + "." + string(cols[i].columnName);
                        if (bstIntIndexes.count(mapKey) || btreeIntIndexes.count(mapKey)) {
                            int v = stoi(vals[i]);
                            // Eliminar la entrada vieja y reinsertar con la nueva posición
                            removeFromIndex(mapKey, v);
                            if (bstIntIndexes.count(mapKey))
                                bstIntIndexes[mapKey]->insert(v, newPos);
                            if (btreeIntIndexes.count(mapKey))
                                btreeIntIndexes[mapKey]->insert(v, newPos);
                        }
                    }
                }
            }
            newPos += rowSize;
        }
        pos += rowSize;
    }

    delete[] buf;
    inFile.close();
    outFile.close();

    // Reemplazar el archivo original con el temporal
    remove(path.c_str());
    rename(tmpPath.c_str(), path.c_str());

    result.success = true;
    result.message = to_string(deletedCount) + " fila(s) eliminada(s).";
    return result;
}
