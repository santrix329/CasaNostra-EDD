#include "mafia_family.h"

#include <fstream>
#include <sstream>
#include <iostream>

// Constructor del nodo: nace con sus datos y sin subordinados (punteros nulos).
MafiaFamily::MafiaNode::MafiaNode(int id, const std::string& name,
                                  const std::string& last_name, char gender,
                                  int age, int id_boss, bool is_dead, bool in_jail,
                                  bool was_boss, bool is_boss)
    : id(id), name(name), last_name(last_name), gender(gender), age(age),
      id_boss(id_boss), is_dead(is_dead), in_jail(in_jail), was_boss(was_boss),
      is_boss(is_boss), left(nullptr), right(nullptr) {}

// El arbol nace vacio.
MafiaFamily::MafiaFamily() : root(nullptr) {}

// Al destruirse la familia se libera todo el arbol de forma automatica (RAII).
MafiaFamily::~MafiaFamily() {
    destroy(root);
}

// Recorrido en postorden: primero los subarboles, luego el nodo actual.
void MafiaFamily::destroy(MafiaNode* node) {
    if (node == nullptr) {
        return;
    }
    destroy(node->left);
    destroy(node->right);
    delete node;
}

// Busca un miembro por id (recorrido en preorden). Devuelve nullptr si no existe.
MafiaFamily::MafiaNode* MafiaFamily::findById(MafiaNode* node, int id) const {
    if (node == nullptr) {
        return nullptr;
    }
    if (node->id == id) {
        return node;
    }
    MafiaNode* found = findById(node->left, id);
    if (found != nullptr) {
        return found;
    }
    return findById(node->right, id);
}

// Cuelga un nodo nuevo de su jefe. El primer subordinado ocupa la izquierda y
// el segundo la derecha (el orden de llegada define la prioridad de sucesion).
void MafiaFamily::attachToBoss(MafiaNode* newNode) {
    MafiaNode* boss = findById(root, newNode->id_boss);
    if (boss == nullptr) {
        std::cerr << "Aviso: no se hallo el jefe (id_boss=" << newNode->id_boss
                  << ") del miembro " << newNode->id << ". Se descarta.\n";
        delete newNode;
        return;
    }
    if (boss->left == nullptr) {
        boss->left = newNode;
    } else if (boss->right == nullptr) {
        boss->right = newNode;
    } else {
        std::cerr << "Aviso: el jefe " << boss->id << " ya tiene dos subordinados; "
                  << "se descarta al miembro " << newNode->id << ".\n";
        delete newNode;
    }
}

// Lee el CSV linea por linea y arma el arbol. Se asume que cada jefe aparece
// antes que sus subordinados (orden natural del archivo de datos).
bool MafiaFamily::loadFromCsv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo '" << path << "'.\n";
        return false;
    }

    std::string line;
    std::getline(file, line);  // descartar la cabecera

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream row(line);
        std::string field;

        std::getline(row, field, ',');
        int id = std::stoi(field);
        std::string name;
        std::getline(row, name, ',');
        std::string last_name;
        std::getline(row, last_name, ',');
        std::getline(row, field, ',');
        char gender = field.empty() ? '?' : field[0];
        std::getline(row, field, ',');
        int age = std::stoi(field);
        std::getline(row, field, ',');
        int id_boss = std::stoi(field);
        std::getline(row, field, ',');
        bool is_dead = (std::stoi(field) == 1);
        std::getline(row, field, ',');
        bool in_jail = (std::stoi(field) == 1);
        std::getline(row, field, ',');
        bool was_boss = (std::stoi(field) == 1);
        std::getline(row, field, ',');
        bool is_boss = (std::stoi(field) == 1);

        MafiaNode* node = new MafiaNode(id, name, last_name, gender, age, id_boss,
                                        is_dead, in_jail, was_boss, is_boss);

        if (id_boss == 0) {
            root = node;         // raiz del arbol (jefe fundador de la estructura)
        } else {
            attachToBoss(node);  // cuelga del jefe correspondiente
        }
    }

    file.close();
    return true;
}

// Cantidad de miembros del subarbol.
int MafiaFamily::countMembers(MafiaNode* node) const {
    if (node == nullptr) {
        return 0;
    }
    return 1 + countMembers(node->left) + countMembers(node->right);
}

// Cantidad total de miembros del arbol.
int MafiaFamily::countMembers() const {
    return countMembers(root);
}
