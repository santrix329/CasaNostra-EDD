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

// Devuelve el jefe actual (nodo con is_boss == 1). Recorrido en preorden.
MafiaFamily::MafiaNode* MafiaFamily::findCurrentBoss(MafiaNode* node) const {
    if (node == nullptr) {
        return nullptr;
    }
    if (node->is_boss) {
        return node;
    }
    MafiaNode* found = findCurrentBoss(node->left);
    if (found != nullptr) {
        return found;
    }
    return findCurrentBoss(node->right);
}

// Un candidato es elegible si esta vivo, libre y dentro del limite de edad.
bool MafiaFamily::isEligible(MafiaNode* node) const {
    return node != nullptr && !node->is_dead && !node->in_jail
           && node->age <= AGE_LIMIT;
}

// Altura del subarbol: cantidad de niveles desde node hacia abajo.
int MafiaFamily::treeHeight(MafiaNode* node) const {
    if (node == nullptr) {
        return 0;
    }
    int leftHeight = treeHeight(node->left);
    int rightHeight = treeHeight(node->right);
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

// Imprime una linea con los datos y el estado de un miembro.
void MafiaFamily::printMemberLine(MafiaNode* node) const {
    std::cout << node->name << " " << node->last_name
              << " (id " << node->id << ", " << node->age << " anios, "
              << node->gender << ")";
    if (node->is_boss) {
        std::cout << " [JEFE]";
    }
    if (node->is_dead) {
        std::cout << " [MUERTO]";
    }
    if (node->in_jail) {
        std::cout << " [PRESO]";
    }
    if (node->age > AGE_LIMIT) {
        std::cout << " [+70]";
    }
    std::cout << "\n";
}

// Imprime los miembros elegibles ubicados exactamente a la profundidad indicada,
// recorriendo de izquierda a derecha (respeta el orden de sucesion).
void MafiaFamily::printEligibleAtDepth(MafiaNode* node, int depth, int& order) const {
    if (node == nullptr) {
        return;
    }
    if (depth == 0) {
        if (isEligible(node)) {
            ++order;
            std::cout << "  " << order << ". ";
            printMemberLine(node);
        }
        return;
    }
    printEligibleAtDepth(node->left, depth - 1, order);
    printEligibleAtDepth(node->right, depth - 1, order);
}

// Lista los sucesores del subarbol del jefe, generacion por generacion
// (los subordinados directos primero, luego la siguiente generacion, etc.).
void MafiaFamily::printSuccessorsByGeneration(MafiaNode* boss) const {
    int height = treeHeight(boss);
    int order = 0;
    for (int depth = 1; depth < height; ++depth) {
        printEligibleAtDepth(boss, depth, order);
    }
    if (order == 0) {
        std::cout << "  (no hay sucesores elegibles en su linea)\n";
    }
}

// Muestra la linea de sucesion actual a partir del jefe.
void MafiaFamily::showSuccessionLine() const {
    MafiaNode* boss = findCurrentBoss(root);
    std::cout << "=== Linea de sucesion actual ===\n";
    if (boss == nullptr) {
        std::cout << "No hay un jefe asignado actualmente.\n";
        return;
    }
    std::cout << "Jefe actual: ";
    printMemberLine(boss);
    std::cout << "Sucesores (vivos, libres y dentro del limite de edad):\n";
    printSuccessorsByGeneration(boss);
}
