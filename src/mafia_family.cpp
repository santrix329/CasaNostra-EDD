#include "mafia_family.h"

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
