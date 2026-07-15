#ifndef MAFIA_FAMILY_H
#define MAFIA_FAMILY_H

#include <string>

// Clase de dominio: representa a la familia mafiosa como un arbol binario de
// jerarquia, donde cada miembro tiene como maximo dos subordinados. El nodo es
// un detalle interno de la clase (struct privado) y solo se manipula a traves
// de los metodos publicos.
class MafiaFamily {
private:
    // Nodo interno del arbol: guarda los datos de un miembro y sus subordinados.
    struct MafiaNode {
        int id;
        std::string name;
        std::string last_name;
        char gender;       // 'H' (hombre) o 'M' (mujer)
        int age;
        int id_boss;       // id del jefe directo; 0 indica la raiz del arbol
        bool is_dead;
        bool in_jail;
        bool was_boss;
        bool is_boss;
        MafiaNode* left;   // primer subordinado
        MafiaNode* right;  // segundo subordinado

        MafiaNode(int id, const std::string& name, const std::string& last_name,
                  char gender, int age, int id_boss, bool is_dead, bool in_jail,
                  bool was_boss, bool is_boss);
    };

    MafiaNode* root;   // raiz del arbol

    // Libera recursivamente toda la memoria del arbol (usado por el destructor).
    void destroy(MafiaNode* node);

public:
    MafiaFamily();
    ~MafiaFamily();
};

#endif
