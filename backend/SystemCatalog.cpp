#include "SystemCatalog.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <stdio.h>
#include "BST.h"
#include "Btree.h"
#include <string.h>
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;
map<string, BST<int>*>    bstIntIndexes;
map<string, BTree<int>*>  btreeIntIndexes;
void initializeSystemCatalog() {
    if (!fs::exists("SystemCatalog")) {
        fs::create_directory("SystemCatalog");
    }

    if (!fs::exists("SystemCatalog/SystemDatabases.bin")) {
        fstream file;
        file.open("SystemCatalog/SystemDatabases.bin", ios::out | ios::binary);
        file.close();
    }

    if (!fs::exists("SystemCatalog/SystemTables.bin")) {
        fstream file;
        file.open("SystemCatalog/SystemTables.bin", ios::out | ios::binary);
        file.close();
    }

    if (!fs::exists("SystemCatalog/SystemColumns.bin")) {
        fstream file;
        file.open("SystemCatalog/SystemColumns.bin", ios::out | ios::binary);
        file.close();
    }

    if (!fs::exists("SystemCatalog/SystemIndexes.bin")) {
        fstream file;
        file.open("SystemCatalog/SystemIndexes.bin", ios::out | ios::binary);
        file.close();
    }
}

void createDatabase(const string& name){
    if (databaseExists(name)) {
        cout << "Error: la base de datos " << name << " ya existe." << endl;
        return;
    }

    if (!fs::exists(name)) {
        fs::create_directory(name);
    }
    DatabaseRecord record;
    strcpy(record.name, name.c_str());
    record.isActive = true;
    
    fstream file;
    file.open("SystemCatalog/SystemDatabases.bin", ios::out | ios::binary | ios::app);
    file.write(reinterpret_cast<char*>(&record), sizeof(DatabaseRecord));
    file.close();
}

bool databaseExists(const string& name) {
    fstream file;
    file.open("SystemCatalog/SystemDatabases.bin", ios::in | ios::binary);
    
    DatabaseRecord record;
    while (file.read(reinterpret_cast<char*>(&record), sizeof(DatabaseRecord))) {
        if (string(record.name) == name) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}


bool tableExists(const string& databaseName, const string& tableName) {
    fstream file;
    file.open("SystemCatalog/SystemTables.bin", ios::in | ios::binary);
    
    TableRecord record;
    while (file.read(reinterpret_cast<char*>(&record), sizeof(TableRecord))) {
        if (string(record.databaseName) == databaseName && string(record.tableName) == tableName) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}
bool getTableRecord(const string& databaseName, const string& tableName, TableRecord& outRecord) {
    fstream file;
    file.open("SystemCatalog/SystemTables.bin", ios::in | ios::binary);

    TableRecord record;
    while (file.read(reinterpret_cast<char*>(&record), sizeof(TableRecord))) {
        if (string(record.databaseName) == databaseName && string(record.tableName) == tableName) {
            outRecord = record;
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void createTable(const string& databaseName, const string& tableName, int columnCount) {
    if (!databaseExists(databaseName)) {
        cout << "Error: la base de datos " << databaseName << " no existe." << endl;
        return;
    }

    if (tableExists(databaseName, tableName)) {
        cout << "Error: la tabla " << tableName << " ya existe en la base de datos " << databaseName << "." << endl;
        return;
    }

    string tablePath = databaseName + "/" + tableName + ".bin";
    if (!fs::exists(tablePath)) {
        fstream tableFile;
        tableFile.open(tablePath, ios::out | ios::binary);
        tableFile.close();
}
    TableRecord record;
    strcpy(record.databaseName, databaseName.c_str());
    strcpy(record.tableName, tableName.c_str());
    record.columnCount = columnCount;

    fstream file;
    file.open("SystemCatalog/SystemTables.bin", ios::out | ios::binary | ios::app);
    file.write(reinterpret_cast<char*>(&record), sizeof(TableRecord));
    file.close();
}

bool columnExists(const string& databaseName, const string& tableName, const string& columnName) {
    fstream file;
    file.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);
    
    ColumnRecord record;
    while(file.read(reinterpret_cast<char*>(&record),sizeof(ColumnRecord))) {
        if (string(record.databaseName)==databaseName && string(record.tableName)==tableName && string(record.columnName)== columnName){
            file.close();
            return true;
        }
}
file.close();
    return false;
}


void addColumn(const string& databaseName, const string& tableName, const string& columnName, const string& dataType, int maxSize, int position) {
    if (!tableExists(databaseName, tableName)) {
        cout << "Error: la tabla " << tableName << " no existe." << endl;
        return;
    }
    if (columnExists(databaseName, tableName, columnName)) {
        cout << "Error: la columna " << columnName << " ya existe." << endl;
        return;
    }
    ColumnRecord record;
    strcpy(record.databaseName, databaseName.c_str());
    strcpy(record.tableName, tableName.c_str());
    strcpy(record.columnName, columnName.c_str());
    strcpy(record.dataType, dataType.c_str());
    record.maxSize = maxSize;
    record.position = position;

    fstream file;
    file.open("SystemCatalog/SystemColumns.bin", ios::out | ios::binary | ios::app);
    file.write(reinterpret_cast<char*>(&record),sizeof(ColumnRecord));
    file.close();
}


bool indexExists(const string& databaseName, const string& tableName, const string& columnName) {
    fstream file;
    file.open("SystemCatalog/SystemIndexes.bin", ios::in | ios::binary);
    
    IndexRecord record;
    while(file.read(reinterpret_cast<char*>(&record), sizeof(IndexRecord))) {
        if (string(record.databaseName) == databaseName && 
            string(record.tableName) == tableName && 
            string(record.columnName) == columnName) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void addIndex(const string& databaseName, const string& tableName, const string& columnName, const string& indexName, const string& indexType) {
    // Verificar que la tabla exista
    if (!tableExists(databaseName, tableName)) {
        cout << "Error: la tabla " << tableName << " no existe." << endl;
        return;
    }

    // Verificar que la columna exista
    if (!columnExists(databaseName, tableName, columnName)) {
        cout << "Error: la columna " << columnName << " no existe." << endl;
        return;
    }

    // Verificar que no haya ya un índice en esa columna
    if (indexExists(databaseName, tableName, columnName)) {
        cout << "Error: ya existe un índice en la columna " << columnName << "." << endl;
        return;
    }

    // Llenar y escribir el record
    IndexRecord record;
    strcpy(record.indexName, indexName.c_str());
    strcpy(record.databaseName, databaseName.c_str());
    strcpy(record.tableName, tableName.c_str());
    strcpy(record.columnName, columnName.c_str());
    strcpy(record.indexType, indexType.c_str());

    fstream file;
    file.open("SystemCatalog/SystemIndexes.bin", ios::out | ios::binary | ios::app);
    file.write(reinterpret_cast<char*>(&record), sizeof(IndexRecord));
    file.close();
}

int calculateRowSize(const string& databaseName, const string& tableName) {
    TableRecord tableRecord;
    if (!getTableRecord(databaseName, tableName, tableRecord)) {
        cout << "Error: la tabla " << tableName << " no existe." << endl;
        return -1;
    }

    int currentColumns = countColumns(databaseName, tableName);
    if (currentColumns != tableRecord.columnCount) {
        cout << "Error: la tabla " << tableName << " tiene " << currentColumns
             << " de " << tableRecord.columnCount << " columnas definidas. "
             << "No se puede calcular el tamaño de fila hasta que la tabla esté completa." << endl;
        return -1;
    }
    
    fstream file;
    file.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);

    int totalSize = 0;
    ColumnRecord record;

    while (file.read(reinterpret_cast<char*>(&record), sizeof(ColumnRecord))) {
        if (string(record.databaseName) == databaseName && 
            string(record.tableName) == tableName) {

            string type = string(record.dataType);

            if (type == "INTEGER") {
                totalSize += sizeof(int);
            } else if (type == "DOUBLE") {
                totalSize += sizeof(double);
            } else if (type == "VARCHAR") {
                totalSize += record.maxSize;
            } else if (type == "DATETIME") {
                totalSize += sizeof(DateTime);
            }
        }
    }

    file.close();
    return totalSize;
}

int countColumns(const string& databaseName, const string& tableName) {
    fstream file;
    file.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);

    int count = 0;
    ColumnRecord record;

    while (file.read(reinterpret_cast<char*>(&record), sizeof(ColumnRecord))) {
        if (string(record.databaseName) == databaseName &&
            string(record.tableName) == tableName) {
            count++;
        }
    }

    file.close();
    return count;
}

void loadIndexes() {
    cout << "📂 ENTRANDO A loadIndexes()" << endl;  // <--- NUEVO LOG
    
    fstream indexFile;
    indexFile.open("SystemCatalog/SystemIndexes.bin", ios::in | ios::binary);
    
    if (!indexFile.is_open()) {
        cout << "   ❌ No se pudo abrir SystemIndexes.bin" << endl;
        return;
    }
    
    cout << "   ✅ Archivo abierto correctamente" << endl;  // <--- NUEVO LOG
    
    // Verificar tamaño
    indexFile.seekg(0, ios::end);
    long fileSize = indexFile.tellg();
    indexFile.seekg(0, ios::beg);
    cout << "   📄 Tamaño: " << fileSize << " bytes" << endl;  // <--- NUEVO LOG
    IndexRecord indexRecord;
    while (indexFile.read(reinterpret_cast<char*>(&indexRecord), sizeof(IndexRecord))) {
        string dbName    = string(indexRecord.databaseName);
        string tableName = string(indexRecord.tableName);
        string colName   = string(indexRecord.columnName);
        string indexType = string(indexRecord.indexType);
        string mapKey    = dbName + "." + tableName + "." + colName;

        // Buscar qué tipo de dato tiene esa columna
        fstream colFile;
        colFile.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);
        ColumnRecord colRecord;
        string dataType = "";
        while (colFile.read(reinterpret_cast<char*>(&colRecord), sizeof(ColumnRecord))) {
            if (string(colRecord.databaseName) == dbName &&
                string(colRecord.tableName)    == tableName &&
                string(colRecord.columnName)   == colName) {
                dataType = string(colRecord.dataType);
                break;
            }
        }
        colFile.close();

        if (dataType == "") {
            cout << "Advertencia: no se encontró la columna " << colName << " al cargar índice." << endl;
            continue;
        }

        // Por ahora solo manejamos INTEGER
        if (dataType != "INTEGER") {
            cout << "Advertencia: índice sobre columna no-INTEGER ignorado por ahora (" << colName << ")." << endl;
            continue;
        }

        // Crear el árbol en memoria según el tipo de índice
        if (indexType == "BST") {
            bstIntIndexes[mapKey] = new BST<int>();
        } else if (indexType == "BTREE") {
            btreeIntIndexes[mapKey] = new BTree<int>();
        }

        // Leer el archivo .bin de la tabla y reconstruir el árbol
        int rowSize = calculateRowSize(dbName, tableName);
        cout << "   rowSize calculado: " << rowSize << endl;
if (rowSize <= 0) {
    cout << "   ⚠️  rowSize inválido, saltando índice" << endl;
    continue;
}
        if (rowSize <= 0) continue;

        string tablePath = dbName + "/" + tableName + ".bin";
        fstream tableFile;
        tableFile.open(tablePath, ios::in | ios::binary);

        // Buscar la posición de la columna indexada dentro de la fila
        // Para eso recorremos las columnas en orden de posición
        fstream colFile2;
        colFile2.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);
        int colOffset = 0; // offset en bytes dentro de la fila donde está la columna indexada
        ColumnRecord cr;
        vector<ColumnRecord> cols;
        while (colFile2.read(reinterpret_cast<char*>(&cr), sizeof(ColumnRecord))) {
            if (string(cr.databaseName) == dbName && string(cr.tableName) == tableName) {
                cols.push_back(cr);
            }
        }
        colFile2.close();

        // Ordenar columnas por posición para calcular el offset correcto
        sort(cols.begin(), cols.end(), [](const ColumnRecord& a, const ColumnRecord& b) {
            return a.position < b.position;
        });

        // Calcular offset sumando tamaños de columnas anteriores
        colOffset = 0;
        for (auto& c : cols) {
            if (string(c.columnName) == colName) break;
            string t = string(c.dataType);
            if      (t == "INTEGER")  colOffset += sizeof(int);
            else if (t == "DOUBLE")   colOffset += sizeof(double);
            else if (t == "VARCHAR")  colOffset += c.maxSize;
            else if (t == "DATETIME") colOffset += sizeof(DateTime);
        }

        // Leer cada fila y extraer el valor de la columna indexada
        long filePos = 0;
        char* rowBuffer = new char[rowSize];
        while (tableFile.read(rowBuffer, rowSize)) {
            xorEncryptDecrypt(rowBuffer, rowSize);
            int value;
            memcpy(&value, rowBuffer + colOffset, sizeof(int));

            if (indexType == "BST") {
                bstIntIndexes[mapKey]->insert(value, filePos);
            } else if (indexType == "BTREE") {
                btreeIntIndexes[mapKey]->insert(value, filePos);
            }
            filePos += rowSize;
        }
        delete[] rowBuffer;
        tableFile.close();

        cout << "Índice cargado: " << mapKey << " (" << indexType << ")" << endl;
    }
    indexFile.close();
}


bool insertRow(const string& databaseName, const string& tableName, const vector<RowValue>& values) {
    // 1. Verificar que la tabla existe y está completa
    int rowSize = calculateRowSize(databaseName, tableName);
    if (rowSize <= 0) {
        cout << "Error: No se pudo calcular el tamaño de fila" << endl;
        return false;
    }

    // 2. Obtener las columnas en orden de posición
    fstream colFile;
    colFile.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);
    vector<ColumnRecord> cols;
    ColumnRecord cr;
    while (colFile.read(reinterpret_cast<char*>(&cr), sizeof(ColumnRecord))) {
        if (string(cr.databaseName) == databaseName && string(cr.tableName) == tableName) {
            cols.push_back(cr);
        }
    }
    colFile.close();

    sort(cols.begin(), cols.end(), [](const ColumnRecord& a, const ColumnRecord& b) {
        return a.position < b.position;
    });

    // 3. Verificar que llegan todos los valores necesarios
    if ((int)values.size() != (int)cols.size()) {
        cout << "Error: se esperaban " << cols.size() << " valores pero llegaron " << values.size() << endl;
        return false;
    }

    // 4. Construir el buffer binario de la fila
    char* rowBuffer = new char[rowSize];
    int offset = 0;

    for (int i = 0; i < (int)cols.size(); i++) {
        string type = string(cols[i].dataType);
        string val  = values[i].value;

        if (type == "INTEGER") {
            int intVal = stoi(val);

            // Verificar duplicado en índice si existe
            string mapKey = databaseName + "." + tableName + "." + string(cols[i].columnName);
            
            // Verificar en BST
            if (bstIntIndexes.count(mapKey)) {
                long foundPos = bstIntIndexes[mapKey]->search(intVal);
                if (foundPos != -1) {
                    cout << "Error: valor duplicado en columna indexada " << cols[i].columnName << " (BST)" << endl;
                    delete[] rowBuffer;
                    return false;
                }
            }
            
            // Verificar en BTree
            if (btreeIntIndexes.count(mapKey)) {
                long foundPos = btreeIntIndexes[mapKey]->search(intVal);
                if (foundPos != -1) {
                    cout << "Error: valor duplicado en columna indexada " << cols[i].columnName << " (BTREE)" << endl;
                    delete[] rowBuffer;
                    return false;
                }
            }

            memcpy(rowBuffer + offset, &intVal, sizeof(int));
            offset += sizeof(int);

        } else if (type == "DOUBLE") {
            double dblVal = stod(val);
            memcpy(rowBuffer + offset, &dblVal, sizeof(double));
            offset += sizeof(double);

        } else if (type == "VARCHAR") {
            memset(rowBuffer + offset, 0, cols[i].maxSize);
            int copyLen = min((int)val.size(), cols[i].maxSize);
            memcpy(rowBuffer + offset, val.c_str(), copyLen);
            offset += cols[i].maxSize;

        } else if (type == "DATETIME") {
            DateTime dt;
            sscanf(val.c_str(), "%d-%d-%d %d:%d:%d",
                &dt.year, &dt.month, &dt.day,
                &dt.hour, &dt.minute, &dt.second);
            memcpy(rowBuffer + offset, &dt, sizeof(DateTime));
            offset += sizeof(DateTime);
        }
    }

    // 4.5 Guardar valores INTEGER ANTES de encriptar para actualizar índices después
    vector<pair<string, int>> intValuesToIndex;
    offset = 0;
    for (int i = 0; i < (int)cols.size(); i++) {
        string type   = string(cols[i].dataType);
        string mapKey = databaseName + "." + tableName + "." + string(cols[i].columnName);
        if (type == "INTEGER") {
            int intVal;
            memcpy(&intVal, rowBuffer + offset, sizeof(int));
            intValuesToIndex.push_back({mapKey, intVal});
            offset += sizeof(int);
        } else if (type == "DOUBLE") {
            offset += sizeof(double);
        } else if (type == "VARCHAR") {
            offset += cols[i].maxSize;
        } else if (type == "DATETIME") {
            offset += sizeof(DateTime);
        }
    }

    // 5. Obtener posición actual ANTES de escribir
    string tablePath = databaseName + "/" + tableName + ".bin";
    fstream tableFile;
    tableFile.open(tablePath, ios::in | ios::binary | ios::ate);
    long filePos = tableFile.tellg();
    tableFile.close();

    // Encriptar y escribir
    xorEncryptDecrypt(rowBuffer, rowSize);
    tableFile.open(tablePath, ios::out | ios::binary | ios::app);
    tableFile.write(rowBuffer, rowSize);
    tableFile.close();

    // 6. Actualizar índices en memoria con valores guardados antes de encriptar
    for (auto& p : intValuesToIndex) {
        string mapKey = p.first;
        int intVal    = p.second;
        
        if (bstIntIndexes.count(mapKey)) {
            if (!bstIntIndexes[mapKey]->insert(intVal, filePos)) {
                cout << "Error: duplicado en BST para " << mapKey << endl;
            }
        }
        if (btreeIntIndexes.count(mapKey)) {
            if (!btreeIntIndexes[mapKey]->insert(intVal, filePos)) {
                cout << "Error: duplicado en BTree para " << mapKey << endl;
            }
        }
    }

    delete[] rowBuffer;
    cout << "Fila insertada correctamente en " << tableName << endl;
    return true;
}
void xorEncryptDecrypt(char* buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] ^= XOR_KEY;
    }
}

void dropTable(const string& databaseName, const string& tableName) {
    // 1. Verificar que la tabla existe
    if (!tableExists(databaseName, tableName)) {
        cout << "Error: la tabla " << tableName << " no existe." << endl;
        return;
    }

    // 2. Verificar que la tabla está vacía
    string tablePath = databaseName + "/" + tableName + ".bin";
    fstream tableFile;
    tableFile.open(tablePath, ios::in | ios::binary | ios::ate);
    long fileSize = tableFile.tellg();
    tableFile.close();

    if (fileSize > 0) {
        cout << "Error: la tabla " << tableName << " no está vacía. No se puede eliminar." << endl;
        return;
    }

    // 3. Eliminar el archivo .bin de la tabla
    fs::remove(tablePath);

    // 4. Reescribir SystemTables.bin sin esta tabla
    fstream inFile, outFile;
    inFile.open("SystemCatalog/SystemTables.bin", ios::in | ios::binary);
    outFile.open("SystemCatalog/SystemTables_temp.bin", ios::out | ios::binary);
    TableRecord tr;
    while (inFile.read(reinterpret_cast<char*>(&tr), sizeof(TableRecord))) {
        if (!(string(tr.databaseName) == databaseName && string(tr.tableName) == tableName)) {
            outFile.write(reinterpret_cast<char*>(&tr), sizeof(TableRecord));
        }
    }
    inFile.close();
    outFile.close();
    fs::remove("SystemCatalog/SystemTables.bin");
    fs::rename("SystemCatalog/SystemTables_temp.bin", "SystemCatalog/SystemTables.bin");

    // 5. Reescribir SystemColumns.bin sin las columnas de esta tabla
    inFile.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);
    outFile.open("SystemCatalog/SystemColumns_temp.bin", ios::out | ios::binary);
    ColumnRecord cr;
    while (inFile.read(reinterpret_cast<char*>(&cr), sizeof(ColumnRecord))) {
        if (!(string(cr.databaseName) == databaseName && string(cr.tableName) == tableName)) {
            outFile.write(reinterpret_cast<char*>(&cr), sizeof(ColumnRecord));
        }
    }
    inFile.close();
    outFile.close();
    fs::remove("SystemCatalog/SystemColumns.bin");
    fs::rename("SystemCatalog/SystemColumns_temp.bin", "SystemCatalog/SystemColumns.bin");

    // 6. Reescribir SystemIndexes.bin sin los índices de esta tabla
    inFile.open("SystemCatalog/SystemIndexes.bin", ios::in | ios::binary);
    outFile.open("SystemCatalog/SystemIndexes_temp.bin", ios::out | ios::binary);
    IndexRecord ir;
    while (inFile.read(reinterpret_cast<char*>(&ir), sizeof(IndexRecord))) {
        if (!(string(ir.databaseName) == databaseName && string(ir.tableName) == tableName)) {
            outFile.write(reinterpret_cast<char*>(&ir), sizeof(IndexRecord));
        }
    }
    inFile.close();
    outFile.close();
    fs::remove("SystemCatalog/SystemIndexes.bin");
    fs::rename("SystemCatalog/SystemIndexes_temp.bin", "SystemCatalog/SystemIndexes.bin");

    // 7. Eliminar índices de memoria
    for (auto it = bstIntIndexes.begin(); it != bstIntIndexes.end();) {
        if (it->first.find(databaseName + "." + tableName + ".") == 0) {
            delete it->second;
            it = bstIntIndexes.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = btreeIntIndexes.begin(); it != btreeIntIndexes.end();) {
        if (it->first.find(databaseName + "." + tableName + ".") == 0) {
            delete it->second;
            it = btreeIntIndexes.erase(it);
        } else {
            ++it;
        }
    }

    cout << "Tabla " << tableName << " eliminada correctamente." << endl;
}

void createIndex(const string& databaseName, const string& tableName, const string& columnName, const string& indexName, const string& indexType) {
    // 1. Verificar que la tabla existe
    if (!tableExists(databaseName, tableName)) {
        cout << "Error: la tabla " << tableName << " no existe." << endl;
        return;
    }

    // 2. Verificar que la columna existe
    if (!columnExists(databaseName, tableName, columnName)) {
        cout << "Error: la columna " << columnName << " no existe." << endl;
        return;
    }

    // 3. Verificar que no hay ya un índice en esa columna
    if (indexExists(databaseName, tableName, columnName)) {
        cout << "Error: ya existe un indice en la columna " << columnName << "." << endl;
        return;
    }

    // 4. Obtener tamaño de fila
    int rowSize = calculateRowSize(databaseName, tableName);
    if (rowSize <= 0) return;

    // 5. Obtener columnas en orden de posición
    fstream colFile;
    colFile.open("SystemCatalog/SystemColumns.bin", ios::in | ios::binary);
    vector<ColumnRecord> cols;
    ColumnRecord cr;
    while (colFile.read(reinterpret_cast<char*>(&cr), sizeof(ColumnRecord))) {
        if (string(cr.databaseName) == databaseName && string(cr.tableName) == tableName) {
            cols.push_back(cr);
        }
    }
    colFile.close();

    sort(cols.begin(), cols.end(), [](const ColumnRecord& a, const ColumnRecord& b) {
        return a.position < b.position;
    });

    // 6. Calcular offset de la columna indexada
    int colOffset = 0;
    string dataType = "";
    for (auto& c : cols) {
        if (string(c.columnName) == columnName) {
            dataType = string(c.dataType);
            break;
        }
        string t = string(c.dataType);
        if      (t == "INTEGER")  colOffset += sizeof(int);
        else if (t == "DOUBLE")   colOffset += sizeof(double);
        else if (t == "VARCHAR")  colOffset += c.maxSize;
        else if (t == "DATETIME") colOffset += sizeof(DateTime);
    }

    // 7. Por ahora solo soportamos INTEGER
    if (dataType != "INTEGER") {
        cout << "Advertencia: por ahora solo se soportan indices sobre columnas INTEGER." << endl;
        return;
    }

    // 8. Leer archivo .bin y verificar que no hay duplicados
    string tablePath = databaseName + "/" + tableName + ".bin";
    fstream tableFile;
    tableFile.open(tablePath, ios::in | ios::binary);

    vector<pair<int, long>> entries; // (valor, posicion)
    char* rowBuffer = new char[rowSize];
    long filePos = 0;

    while (tableFile.read(rowBuffer, rowSize)) {
        xorEncryptDecrypt(rowBuffer, rowSize); // desencriptar
        int value;
        memcpy(&value, rowBuffer + colOffset, sizeof(int));

        // Verificar duplicado
        for (auto& e : entries) {
            if (e.first == value) {
                cout << "Error: la columna " << columnName << " tiene valores duplicados. No se puede crear el indice." << endl;
                delete[] rowBuffer;
                tableFile.close();
                return;
            }
        }
        entries.push_back({value, filePos});
        filePos += rowSize;
    }
    delete[] rowBuffer;
    tableFile.close();

    // 9. Registrar el índice en SystemIndexes.bin
    addIndex(databaseName, tableName, columnName, indexName, indexType);

    // 10. Construir el árbol en memoria
    string mapKey = databaseName + "." + tableName + "." + columnName;
    if (indexType == "BST") {
        bstIntIndexes[mapKey] = new BST<int>();
        for (auto& e : entries) {
            bstIntIndexes[mapKey]->insert(e.first, e.second);
        }
    } else if (indexType == "BTREE") {
        btreeIntIndexes[mapKey] = new BTree<int>();
        for (auto& e : entries) {
            btreeIntIndexes[mapKey]->insert(e.first, e.second);
        }
    }

    cout << "Indice " << indexName << " creado correctamente sobre " << columnName << " (" << indexType << ")." << endl;
}

