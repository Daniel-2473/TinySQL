#pragma once
#include <vector>
#include <string>

// Representa una fila completa leída del archivo .bin
struct Row {
    std::vector<std::string> values; // cada columna de la fila como string
    long position;                   // posición en el archivo .bin donde está esta fila
};

// Función pública que llama el resto del sistema
void sortRows(std::vector<Row>& rows, int columnIndex, const std::string& dataType, bool ascending);