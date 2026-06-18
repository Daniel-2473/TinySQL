#pragma once
#include <vector>
#include <string>
#include <functional>
using namespace std;

// Representa una fila completa leída del archivo .bin
struct Row {
    vector<string> values; // cada columna de la fila como string
    long position;         // posición en el archivo .bin donde está esta fila
};

bool compareValues(const string& a, const string& b, const string& dataType, bool ascending) {
    int result;

    if (dataType == "INTEGER") {
        result = (stoi(a) < stoi(b)) ? -1 : (stoi(a) > stoi(b)) ? 1 : 0;
    } else if (dataType == "DOUBLE") {
        result = (stod(a) < stod(b)) ? -1 : (stod(a) > stod(b)) ? 1 : 0;
    } else {
        // VARCHAR y DATETIME se comparan como string directamente
        result = a.compare(b);
    }

    return ascending ? result < 0 : result > 0;
}

// Particiona el arreglo y devuelve el índice final del pivote
int partition(vector<Row>& rows, int low, int high, int columnIndex, const string& dataType, bool ascending) {
    Row pivot = rows[high]; // el pivote es el último elemento
    int i = low - 1;       // i apunta al último elemento "menor que el pivote"

    for (int j = low; j < high; j++) {
        // Si la fila j debe ir antes que el pivote
        if (compareValues(rows[j].values[columnIndex], pivot.values[columnIndex], dataType, ascending)) {
            i++;
            swap(rows[i], rows[j]);
        }
    }
    // Colocamos el pivote en su posición final
    swap(rows[i + 1], rows[high]);
    return i + 1;
}

// Quicksort recursivo
void quickSort(vector<Row>& rows, int low, int high, int columnIndex, const string& dataType, bool ascending) {
    if (low < high) {
        int pivotIndex = partition(rows, low, high, columnIndex, dataType, ascending);
        quickSort(rows, low, pivotIndex - 1, columnIndex, dataType, ascending);
        quickSort(rows, pivotIndex + 1, high, columnIndex, dataType, ascending);
    }
}

// Función pública que llama el resto del sistema
void sortRows(vector<Row>& rows, int columnIndex, const string& dataType, bool ascending) {
    if (rows.empty()) return;
    quickSort(rows, 0, rows.size() - 1, columnIndex, dataType, ascending);
}