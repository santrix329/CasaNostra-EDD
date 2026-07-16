#ifndef CSV_MANAGER_H
#define CSV_MANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "mafia_node.h"

// Estructura de Lista Enlazada simple para almacenar temporalmente los datos
struct TempNode {
    MafiaNode* data;
    TempNode* next;
};

/*
 * leerCSV
 * Abre el archivo, crea los nodos y los guarda en una lista enlazada.
 * Retorna el puntero al inicio de la lista.
 */
TempNode* leerCSV(string ruta) {
    ifstream archivo(ruta);
    TempNode* head = nullptr;

    if (!archivo.is_open()) return nullptr;

    string linea;
    getline(archivo, linea); // Saltar la cabecera del CSV

    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string dato;
        
        // Variables para guardar los campos leídos
        int id, id_boss, age;
        string name, last_name;
        char gender;
        bool is_dead, in_jail, was_boss, is_boss;

        // Leer los datos separados por comas
        // (Asumiendo el orden exacto del enunciado)
        getline(ss, dato, ','); id = stoi(dato);
        getline(ss, name, ',');
        getline(ss, last_name, ',');
        getline(ss, dato, ','); gender = dato[0];
        getline(ss, dato, ','); age = stoi(dato);
        getline(ss, dato, ','); id_boss = stoi(dato);
        getline(ss, dato, ','); is_dead = stoi(dato);
        getline(ss, dato, ','); in_jail = stoi(dato);
        getline(ss, dato, ','); was_boss = stoi(dato);
        getline(ss, dato, ','); is_boss = stoi(dato);

        // Crear el nodo y agregarlo a la lista temporal
        MafiaNode* nuevo = createMafiaNode(id, name, last_name, gender, age, id_boss, is_dead, in_jail, was_boss, is_boss);
        TempNode* temp = new TempNode{nuevo, head};
        head = temp;
    }
    archivo.close();
    return head;
}

#endif