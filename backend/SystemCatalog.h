#pragma once
#include <string>
#include "BST.h"
#include "BTree.h"
#include <map>
#include <vector>
using namespace std;
extern map<string, BST<int>*>    bstIntIndexes;
extern map<string, BTree<int>*>  btreeIntIndexes;

struct RowValue {
    string columnName;
    string value; 
};
struct DatabaseRecord {
    char name[50];
    bool isActive;
};

struct TableRecord {
    char databaseName[50];
    char tableName[50];
    int columnCount;
};

struct ColumnRecord{
    char databaseName[50];
    char tableName[50];
    char columnName[50];
    char dataType[50];
    int maxSize;
    int position;//posicion para saber en donde esta en la tabla
};

struct IndexRecord{
    char indexName[50];
    char databaseName[50];
    char tableName[50];
    char columnName[50];
    char indexType[10];
};
struct DateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
};
void initializeSystemCatalog();
bool databaseExists(const string& name);
bool tableExists(const string& databaseName, const string& tableName);
void createDatabase(const string& name);
void createTable(const string& databaseName, const string& tableName, int columnCount);
bool columnExists(const string& databaseName, const string& tableName, const string& columnName);
void addColumn(const string& databaseName, const string& tableName, const string& columnName, const string& dataType, int maxSize, int position);
bool indexExists(const string& databaseName, const string& tableName, const string& columnName);
void addIndex(const string& databaseName, const string& tableName, const string& columnName, const string& indexName, const string& indexType);
int calculateRowSize(const string& databaseName, const string& tableName);
int countColumns(const string& databaseName, const string& tableName);
bool getTableRecord(const string& databaseName, const string& tableName, TableRecord& outRecord);
void loadIndexes();
void insertRow(const string& databaseName, const string& tableName, const vector<RowValue>& values);
const char XOR_KEY = 0x5A; // clave secreta
void xorEncryptDecrypt(char* buffer, int size);