#include "Quicksort.h"
#include <algorithm>
using namespace std;

// Interna: compara dos valores como strings según su tipo de dato
static bool compareValues(const string& a, const string& b, const string& dataType, bool ascending) {
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

// Interna: particiona el arreglo y devuelve el índice final del pivote
static int partition(vector<Row>& rows, int low, int high, int columnIndex, const string& dataType, bool ascending) {
    Row pivot = rows[high]; // el pivote es el último elemento
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (compareValues(rows[j].values[columnIndex], pivot.values[columnIndex], dataType, ascending)) {
            i++;
            swap(rows[i], rows[j]);
        }
    }
    swap(rows[i + 1], rows[high]);
    return i + 1;
}

// Interna: quicksort recursivo
static void quickSort(vector<Row>& rows, int low, int high, int columnIndex, const string& dataType, bool ascending) {
    if (low < high) {
        int pivotIndex = partition(rows, low, high, columnIndex, dataType, ascending);
        quickSort(rows, low, pivotIndex - 1, columnIndex, dataType, ascending);
        quickSort(rows, pivotIndex + 1, high, columnIndex, dataType, ascending);
    }
}

// Pública: único punto de entrada para ordenar filas
void sortRows(vector<Row>& rows, int columnIndex, const string& dataType, bool ascending) {
    if (rows.empty()) return;
    quickSort(rows, 0, (int)rows.size() - 1, columnIndex, dataType, ascending);
}
