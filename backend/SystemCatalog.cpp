#include "SystemCatalog.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <stdio.h>
#include "BST.h"
#include "BTree.h"
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
    fstream indexFile;
    indexFile.open("SystemCatalog/SystemIndexes.bin", ios::in | ios::binary);

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