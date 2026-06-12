// models/QueryResult.h
#pragma once
#include <string>

struct QueryResult {
    bool        success;
    std::string message;
    double      executionTimeMs;

    // Columnas: arreglo estático con tamaño máximo razonable
    std::string columns[50];
    int         columnCount = 0;

    // Filas: matriz estática
    std::string rows[1000][50];
    int         rowCount = 0;
};