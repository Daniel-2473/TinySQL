#include "SystemCatalog.h"
#include "BTree.h"
#include "Quicksort.h"
#include <iostream>
using namespace std;

int main() {
    // Inicializar sistema
    initializeSystemCatalog();
    cout << "--- Sistema inicializado ---" << endl;

    // Crear base de datos
    createDatabase("Universidad");
    cout << "--- Base de datos creada ---" << endl;

    // Crear tabla con 5 columnas (igual que el ejemplo del enunciado)
    createTable("Universidad", "Estudiante", 5);
    cout << "--- Tabla creada ---" << endl;

    // Agregar columnas
    addColumn("Universidad", "Estudiante", "ID",               "INTEGER", 0,  0);
    addColumn("Universidad", "Estudiante", "Nombre",           "VARCHAR", 30, 1);
    addColumn("Universidad", "Estudiante", "PrimerApellido",   "VARCHAR", 30, 2);
    addColumn("Universidad", "Estudiante", "SegundoApellido",  "VARCHAR", 30, 3);
    addColumn("Universidad", "Estudiante", "FechaNacimiento",  "DATETIME", 0, 4);
    cout << "--- Columnas agregadas ---" << endl;

    // Calcular tamaño de fila
    int size = calculateRowSize("Universidad", "Estudiante");
    cout << "Tamanio de fila: " << size << " bytes" << endl;

    // Crear índice BTREE sobre ID
    addIndex("Universidad", "Estudiante", "ID", "Estudiante_Id", "BTREE");
    cout << "--- Indice BTREE creado sobre ID ---" << endl;

    // Cargar índices en memoria
    loadIndexes();
    cout << "--- Indices cargados en memoria ---" << endl;

    // Insertar filas válidas
    cout << "\n--- Insertando filas ---" << endl;

    insertRow("Universidad", "Estudiante", {
        {"ID", "1"}, {"Nombre", "Isaac"}, {"PrimerApellido", "Ramirez"},
        {"SegundoApellido", "Herrera"}, {"FechaNacimiento", "2000-01-01 01:02:00"}
    });

    insertRow("Universidad", "Estudiante", {
        {"ID", "2"}, {"Nombre", "Juan"}, {"PrimerApellido", "Ramirez"},
        {"SegundoApellido", "X"}, {"FechaNacimiento", "2000-01-01 01:02:00"}
    });

    insertRow("Universidad", "Estudiante", {
        {"ID", "3"}, {"Nombre", "Pedro"}, {"PrimerApellido", "Herrera"},
        {"SegundoApellido", "Y"}, {"FechaNacimiento", "2000-01-01 01:02:00"}
    });

    // Intentar insertar duplicado (debe dar error)
    cout << "\n--- Intentando insertar duplicado (ID=1) ---" << endl;
    insertRow("Universidad", "Estudiante", {
        {"ID", "1"}, {"Nombre", "Andres"}, {"PrimerApellido", "Ramirez"},
        {"SegundoApellido", "Z"}, {"FechaNacimiento", "2000-01-01 01:02:00"}
    });

    // Intentar insertar tabla incompleta (debe dar error)
    cout << "\n--- Intentando insertar valores incompletos ---" << endl;
    insertRow("Universidad", "Estudiante", {
        {"ID", "4"}, {"Nombre", "Maria"}
    });

    cout << "\n--- Prueba completada ---" << endl;
    return 0;
}