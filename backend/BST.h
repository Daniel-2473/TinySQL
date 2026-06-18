#pragma once
#include <iostream>
using namespace std;

template <typename T>
struct BSTNode {
    T key;
    long position;
    BSTNode* left;
    BSTNode* right;

    BSTNode(T key, long position) {
        this->key = key;
        this->position = position;
        left = nullptr;
        right = nullptr;
    }
};

template <typename T>
class BST {
private:
    BSTNode<T>* root;

    BSTNode<T>* insertHelper(BSTNode<T>* node, T key, long position) {
        if (node == nullptr) {
            return new BSTNode<T>(key, position);
        }
        if (key < node->key) {
            node->left = insertHelper(node->left, key, position);
        } else if (key > node->key) {
            node->right = insertHelper(node->right, key, position);
        } else {
            cout << "Error: la clave ya existe en el indice, no se permite duplicado." << endl;
        }
        return node;
    }

    void inOrderHelper(BSTNode<T>* node) {
        if (node == nullptr) return;
        inOrderHelper(node->left);
        cout << "Key: " << node->key << " -> Position: " << node->position << endl;
        inOrderHelper(node->right);
    }

    long searchHelper(BSTNode<T>* node, T key) {
        if (node == nullptr) {
            return -1;
        }
        if (key < node->key) {
            return searchHelper(node->left, key);
        } else if (key > node->key) {
            return searchHelper(node->right, key);
        } else {
            return node->position;
        }
    }

    BSTNode<T>* findMin(BSTNode<T>* node) {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    BSTNode<T>* removeHelper(BSTNode<T>* node, T key) {
        if (node == nullptr) {
            cout << "Error: la clave no existe en el indice." << endl;
            return nullptr;
        }

        if (key < node->key) {
            node->left = removeHelper(node->left, key);
        } else if (key > node->key) {
            node->right = removeHelper(node->right, key);
        } else {
            // Caso 1: no tiene hijos
            if (node->left == nullptr && node->right == nullptr) {
                delete node;
                return nullptr;
            // Caso 2a: solo tiene hijo derecho
            } else if (node->left == nullptr) {
                BSTNode<T>* temp = node->right;
                delete node;
                return temp;
            // Caso 2b: solo tiene hijo izquierdo
            } else if (node->right == nullptr) {
                BSTNode<T>* temp = node->left;
                delete node;
                return temp;
            // Caso 3: tiene dos hijos
            } else {
                BSTNode<T>* sucesor = findMin(node->right);
                node->key = sucesor->key;
                node->position = sucesor->position;
                node->right = removeHelper(node->right, sucesor->key);
            }
        }
        return node;
    }

public:
    BST() {
        root = nullptr;
    }

    void insert(T key, long position) {
        root = insertHelper(root, key, position);
    }

    long search(T key) {
        return searchHelper(root, key);
    }

    void remove(T key) {
        root = removeHelper(root, key);
    }

    void printInOrder() {
        inOrderHelper(root);
    }
};