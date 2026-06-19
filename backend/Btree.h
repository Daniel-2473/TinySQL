#pragma once
#include <iostream>
using namespace std;

const int ORDER = 3;

template <typename T>
struct BTreeNode {
    int numKeys;
    T keys[2 * ORDER - 1];
    long positions[2 * ORDER - 1];
    BTreeNode* children[2 * ORDER];
    bool isLeaf;

    BTreeNode(bool isLeaf) {
        this->isLeaf = isLeaf;
        this->numKeys = 0;
        for (int i = 0; i < 2 * ORDER; i++) {
            children[i] = nullptr;
        }
    }
};

template <typename T>
class BTree {
private:
    BTreeNode<T>* root;

    // Busca la clave en el subárbol con raíz en 'node'
    long searchHelper(BTreeNode<T>* node, T key) {
        if (node == nullptr) return -1;

        int i = 0;
        // Avanza mientras la clave buscada sea mayor que la clave actual
        while (i < node->numKeys && key > node->keys[i]) {
            i++;
        }

        // Si encontramos la clave exacta
        if (i < node->numKeys && key == node->keys[i]) {
            return node->positions[i];
        }

        // Si es hoja y no la encontramos, no existe
        if (node->isLeaf) return -1;

        // Bajamos al hijo correspondiente
        return searchHelper(node->children[i], key);
    }

    // Divide el hijo 'child' del nodo 'parent' en la posición 'index'
    void splitChild(BTreeNode<T>* parent, int index, BTreeNode<T>* child) {
        // Creamos el nuevo nodo que va a recibir la mitad derecha
        BTreeNode<T>* newNode = new BTreeNode<T>(child->isLeaf);
        newNode->numKeys = ORDER - 1;

        // Copiamos las claves de la mitad derecha al nuevo nodo
        for (int i = 0; i < ORDER - 1; i++) {
            newNode->keys[i] = child->keys[i + ORDER];
            newNode->positions[i] = child->positions[i + ORDER];
        }

        // Si el hijo no es hoja, también copiamos sus hijos
        if (!child->isLeaf) {
            for (int i = 0; i < ORDER; i++) {
                newNode->children[i] = child->children[i + ORDER];
            }
        }

        // El hijo original ahora solo tiene ORDER-1 claves
        child->numKeys = ORDER - 1;

        // Hacemos espacio en el padre para el nuevo hijo
        for (int i = parent->numKeys; i >= index + 1; i--) {
            parent->children[i + 1] = parent->children[i];
        }
        parent->children[index + 1] = newNode;

        // Subimos la clave del medio al padre
        for (int i = parent->numKeys - 1; i >= index; i--) {
            parent->keys[i + 1] = parent->keys[i];
            parent->positions[i + 1] = parent->positions[i];
        }
        parent->keys[index] = child->keys[ORDER - 1];
        parent->positions[index] = child->positions[ORDER - 1];
        parent->numKeys++;
    }

    // Inserta en un nodo que garantizamos que NO está lleno
    void insertNonFull(BTreeNode<T>* node, T key, long position) {
        int i = node->numKeys - 1;

        if (node->isLeaf) {
            // Desplazamos claves mayores hacia la derecha para hacer espacio
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                node->positions[i + 1] = node->positions[i];
                i--;
            }
            // Detectamos duplicado
            if (i >= 0 && key == node->keys[i]) {
                cout << "Error: la clave ya existe en el indice, no se permite duplicado." << endl;
                return;
            }
            // Insertamos la nueva clave
            node->keys[i + 1] = key;
            node->positions[i + 1] = position;
            node->numKeys++;
        } else {
            // Bajamos al hijo correcto
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            // Verificamos duplicado en este nodo
            if (i >= 0 && key == node->keys[i]) {
                cout << "Error: la clave ya existe en el indice, no se permite duplicado." << endl;
                return;
            }
            i++;
            // Si el hijo está lleno, lo dividimos primero
            if (node->children[i]->numKeys == 2 * ORDER - 1) {
                splitChild(node, i, node->children[i]);
                // Después del split, decidimos a cuál de los dos hijos bajar
                if (key == node->keys[i]) {
                    cout << "Error: la clave ya existe en el indice, no se permite duplicado." << endl;
                    return;
                }
                if (key > node->keys[i]) i++;
            }
            insertNonFull(node->children[i], key, position);
        }
    }

    // Encuentra el predecesor (máximo del subárbol izquierdo)
    BTreeNode<T>* findMaxNode(BTreeNode<T>* node) {
        while (!node->isLeaf) {
            node = node->children[node->numKeys];
        }
        return node;
    }

    // Encuentra el sucesor (mínimo del subárbol derecho)
    BTreeNode<T>* findMinNode(BTreeNode<T>* node) {
        while (!node->isLeaf) {
            node = node->children[0];
        }
        return node;
    }

    // Elimina la clave del subárbol con raíz en 'node'
    void removeHelper(BTreeNode<T>* node, T key) {
        int i = 0;
        while (i < node->numKeys && key > node->keys[i]) i++;

        if (i < node->numKeys && key == node->keys[i]) {
            // La clave está en este nodo
            if (node->isLeaf) {
                // Caso 1: nodo hoja, simplemente eliminamos
                for (int j = i; j < node->numKeys - 1; j++) {
                    node->keys[j] = node->keys[j + 1];
                    node->positions[j] = node->positions[j + 1];
                }
                node->numKeys--;
            } else {
                // Caso 2: nodo interno
                if (node->children[i]->numKeys >= ORDER) {
                    // Caso 2a: el hijo izquierdo tiene suficientes claves
                    BTreeNode<T>* predNode = findMaxNode(node->children[i]);
                    node->keys[i] = predNode->keys[predNode->numKeys - 1];
                    node->positions[i] = predNode->positions[predNode->numKeys - 1];
                    removeHelper(node->children[i], node->keys[i]);
                } else if (node->children[i + 1]->numKeys >= ORDER) {
                    // Caso 2b: el hijo derecho tiene suficientes claves
                    BTreeNode<T>* sucNode = findMinNode(node->children[i + 1]);
                    node->keys[i] = sucNode->keys[0];
                    node->positions[i] = sucNode->positions[0];
                    removeHelper(node->children[i + 1], node->keys[i]);
                } else {
                    // Caso 2c: ambos hijos tienen ORDER-1 claves, los fusionamos
                    mergeChildren(node, i);
                    removeHelper(node->children[i], key);
                }
            }
        } else {
            // La clave no está en este nodo, bajamos al hijo
            if (node->isLeaf) {
                cout << "Error: la clave no existe en el indice." << endl;
                return;
            }
            bool isLastChild = (i == node->numKeys);
            if (node->children[i]->numKeys < ORDER) {
                fixChild(node, i);
                // fixChild puede cambiar la estructura, recalculamos i
                if (isLastChild && i > node->numKeys) i--;
            }
            removeHelper(node->children[i], key);
        }
    }

    // Fusiona el hijo i con el hijo i+1
    void mergeChildren(BTreeNode<T>* parent, int i) {
        BTreeNode<T>* leftChild = parent->children[i];
        BTreeNode<T>* rightChild = parent->children[i + 1];

        // Bajamos la clave del padre al hijo izquierdo
        leftChild->keys[ORDER - 1] = parent->keys[i];
        leftChild->positions[ORDER - 1] = parent->positions[i];

        // Copiamos las claves del hijo derecho al izquierdo
        for (int j = 0; j < rightChild->numKeys; j++) {
            leftChild->keys[ORDER + j] = rightChild->keys[j];
            leftChild->positions[ORDER + j] = rightChild->positions[j];
        }

        // Copiamos los hijos del hijo derecho al izquierdo
        if (!leftChild->isLeaf) {
            for (int j = 0; j <= rightChild->numKeys; j++) {
                leftChild->children[ORDER + j] = rightChild->children[j];
            }
        }

        leftChild->numKeys += rightChild->numKeys + 1;

        // Eliminamos la clave del padre y el puntero al hijo derecho
        for (int j = i; j < parent->numKeys - 1; j++) {
            parent->keys[j] = parent->keys[j + 1];
            parent->positions[j] = parent->positions[j + 1];
            parent->children[j + 1] = parent->children[j + 2];
        }
        parent->numKeys--;

        delete rightChild;
    }

    // Se asegura de que el hijo i tenga al menos ORDER claves antes de bajar
    void fixChild(BTreeNode<T>* parent, int i) {
        if (i > 0 && parent->children[i - 1]->numKeys >= ORDER) {
            // Rotación desde el hermano izquierdo
            BTreeNode<T>* child = parent->children[i];
            BTreeNode<T>* leftSibling = parent->children[i - 1];

            for (int j = child->numKeys; j > 0; j--) {
                child->keys[j] = child->keys[j - 1];
                child->positions[j] = child->positions[j - 1];
            }
            if (!child->isLeaf) {
                for (int j = child->numKeys + 1; j > 0; j--) {
                    child->children[j] = child->children[j - 1];
                }
            }
            child->keys[0] = parent->keys[i - 1];
            child->positions[0] = parent->positions[i - 1];
            if (!child->isLeaf) {
                child->children[0] = leftSibling->children[leftSibling->numKeys];
            }
            parent->keys[i - 1] = leftSibling->keys[leftSibling->numKeys - 1];
            parent->positions[i - 1] = leftSibling->positions[leftSibling->numKeys - 1];
            leftSibling->numKeys--;
            child->numKeys++;

        } else if (i < parent->numKeys && parent->children[i + 1]->numKeys >= ORDER) {
            // Rotación desde el hermano derecho
            BTreeNode<T>* child = parent->children[i];
            BTreeNode<T>* rightSibling = parent->children[i + 1];

            child->keys[child->numKeys] = parent->keys[i];
            child->positions[child->numKeys] = parent->positions[i];
            if (!child->isLeaf) {
                child->children[child->numKeys + 1] = rightSibling->children[0];
            }
            parent->keys[i] = rightSibling->keys[0];
            parent->positions[i] = rightSibling->positions[0];
            for (int j = 0; j < rightSibling->numKeys - 1; j++) {
                rightSibling->keys[j] = rightSibling->keys[j + 1];
                rightSibling->positions[j] = rightSibling->positions[j + 1];
            }
            if (!rightSibling->isLeaf) {
                for (int j = 0; j < rightSibling->numKeys; j++) {
                    rightSibling->children[j] = rightSibling->children[j + 1];
                }
            }
            rightSibling->numKeys--;
            child->numKeys++;

        } else {
            // Fusión
            if (i < parent->numKeys) {
                mergeChildren(parent, i);
            } else {
                mergeChildren(parent, i - 1);
            }
        }
    }

    void inOrderHelper(BTreeNode<T>* node) {
        if (node == nullptr) return;
        for (int i = 0; i < node->numKeys; i++) {
            if (!node->isLeaf) {
                inOrderHelper(node->children[i]);
            }
            cout << "Key: " << node->keys[i] << " -> Position: " << node->positions[i] << endl;
        }
        if (!node->isLeaf) {
            inOrderHelper(node->children[node->numKeys]);
        }
    }

public:
    BTree() {
        root = nullptr;
    }

    long search(T key) {
        return searchHelper(root, key);
    }

    bool insert(T key, long position) {
    if (search(key) != -1) return false; // ya existe
    if (root == nullptr) {
        root = new BTreeNode<T>(true);
        root->keys[0] = key;
        root->positions[0] = position;
        root->numKeys = 1;
        return true;
    }
    if (root->numKeys == 2 * ORDER - 1) {
        BTreeNode<T>* newRoot = new BTreeNode<T>(false);
        newRoot->children[0] = root;
        splitChild(newRoot, 0, root);
        root = newRoot;
    }
    insertNonFull(root, key, position);
    return true;
}

    void remove(T key) {
        if (root == nullptr) {
            cout << "Error: el arbol esta vacio." << endl;
            return;
        }
        removeHelper(root, key);
        // Si la raíz quedó vacía después de una fusión, bajamos la raíz
        if (root->numKeys == 0) {
            BTreeNode<T>* oldRoot = root;
            if (root->isLeaf) {
                root = nullptr;
            } else {
                root = root->children[0];
            }
            delete oldRoot;
        }
    }

    void printInOrder() {
        inOrderHelper(root);
    }
};