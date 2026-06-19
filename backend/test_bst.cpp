#include "BST.h"
#include <iostream>
using namespace std;

int main() {
    BST<int> arbol;

    arbol.insert(50, 100);
    arbol.insert(30, 200);
    arbol.insert(70, 300);
    arbol.insert(20, 400);
    arbol.insert(40, 500);
    arbol.insert(50, 999); // debe dar error: clave duplicada

    cout << "--- Recorrido in-order ---" << endl;
    arbol.printInOrder();

    return 0;
}