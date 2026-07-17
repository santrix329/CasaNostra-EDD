#include "mafia_family.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

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

// convierte una cadena a entero sin lanzar excepciones (a diferencia de std::stoi).
// una cadena vacia o no numerica devuelve 0; corta ante el primer caracter no digito,
// asi ignora espacios sobrantes o un '\r' final en archivos con formato windows.
int MafiaFamily::parseInt(const std::string& field) {
    int i = 0;
    int length = static_cast<int>(field.length());
    while (i < length && field[i] == ' ') {
        i++;
    }
    bool negative = false;
    if (i < length && field[i] == '-') {
        negative = true;
        i++;
    }
    int value = 0;
    while (i < length && field[i] >= '0' && field[i] <= '9') {
        value = value * 10 + (field[i] - '0');
        i++;
    }
    return negative ? -value : value;
}

/*
 * loadFromCsv
 * Lee el archivo CSV desordenado, guarda los miembros en una lista enlazada temporal
 * para respetar la prohibición de vectores y construye el árbol validando las jerarquías.
 */
bool MafiaFamily::loadFromCsv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo '" << path << "'.\n";
        return false;
    }

    // Estructura de Lista Enlazada simple local para almacenar temporalmente los datos
    struct TempNode {
        MafiaNode* data;
        TempNode* next;
    };
    TempNode* head = nullptr;
    TempNode* tail = nullptr;

    std::string line;
    std::getline(file, line);  // descartar la cabecera

    // 1. Leer todo el archivo y meterlo a la lista temporal
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream row(line);
        std::string field;

        std::getline(row, field, ','); int id = parseInt(field);
        std::string name; std::getline(row, name, ',');
        std::string last_name; std::getline(row, last_name, ',');
        std::getline(row, field, ','); char gender = field.empty() ? '?' : field[0];
        std::getline(row, field, ','); int age = parseInt(field);
        std::getline(row, field, ','); int id_boss = parseInt(field);
        std::getline(row, field, ','); bool is_dead = (parseInt(field) == 1);
        std::getline(row, field, ','); bool in_jail = (parseInt(field) == 1);
        std::getline(row, field, ','); bool was_boss = (parseInt(field) == 1);
        std::getline(row, field, ','); bool is_boss = (parseInt(field) == 1);

        MafiaNode* node = new MafiaNode(id, name, last_name, gender, age, id_boss,
                                        is_dead, in_jail, was_boss, is_boss);

        // se agrega al final de la lista para conservar el orden del archivo:
        // el primer subordinado leido queda como hijo izquierdo (prioridad de sucesion)
        TempNode* temp = new TempNode{node, nullptr};
        if (tail == nullptr) {
            head = temp;
        } else {
            tail->next = temp;
        }
        tail = temp;
    }
    file.close();

    // 2. Procesar la lista temporal construyendo el árbol jerárquico
    while (head != nullptr) {
        TempNode* actual = head;
        TempNode* anterior = nullptr;
        bool seInsertoAlguien = false;

        while (actual != nullptr) {
            bool insertado = false;

            if (actual->data->id_boss == 0) {
                if (root == nullptr) {
                    root = actual->data; // Asignar el Jefe Supremo
                    insertado = true;
                }
            } else {
                // Verificar si el jefe ya existe en el árbol antes de pasarlo a attachToBoss
                // Esto evita que attachToBoss elimine el nodo si el jefe aún no ha sido cargado.
                MafiaNode* jefe = findById(root, actual->data->id_boss);
                if (jefe != nullptr) {
                    attachToBoss(actual->data);
                    insertado = true;
                }
            }

            if (insertado) {
                // Desconectar el nodo temporal de la lista
                if (anterior == nullptr) {
                    head = actual->next;
                } else {
                    anterior->next = actual->next;
                }
                TempNode* aBorrar = actual;
                actual = actual->next;
                delete aBorrar; // Solo borramos la caja temporal, el MafiaNode ya está en el árbol
                seInsertoAlguien = true;
            } else {
                // El jefe no está todavía, pasamos al siguiente
                anterior = actual;
                actual = actual->next;
            }
        }

        // Prevención de bucle infinito si hay datos corruptos en el CSV
        if (!seInsertoAlguien) {
            std::cerr << "Advertencia: Algunos miembros no encontraron a su jefe y fueron descartados.\n";
            while (head != nullptr) {
                TempNode* aBorrar = head;
                head = head->next;
                delete aBorrar->data; // Evitar fugas de memoria
                delete aBorrar;
            }
            break;
        }
    }

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
    return isEligible(node, false);
}

// Variante configurable: si allowJailed es true, un preso vivo tambien cuenta
// (regla de ultimo recurso cuando ya no quedan candidatos libres).
bool MafiaFamily::isEligible(MafiaNode* node, bool allowJailed) const {
    if (node == nullptr || node->is_dead || node->age > AGE_LIMIT) {
        return false;
    }
    if (!allowJailed && node->in_jail) {
        return false;
    }
    return true;
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

// muestra la linea de sucesion actual: la cadena de personas que tomarian el
// puesto, en orden, aplicando las condiciones de sucesion sobre el estado actual
// de los miembros (aclaracion del profesor). se proyecta con el mismo algoritmo
// que usa la reasignacion real (findSuccessor: primero libres, luego presos vivos),
// asi el primero de la linea siempre coincide con quien asumiria de verdad.
void MafiaFamily::showSuccessionLine() {
    MafiaNode* boss = findCurrentBoss(root);
    std::cout << "=== Linea de sucesion actual ===\n";
    if (boss == nullptr) {
        std::cout << "No hay un jefe asignado actualmente.\n";
        return;
    }
    std::cout << "Jefe actual: ";
    printMemberLine(boss);
    std::cout << "Quien tomaria el puesto, en orden:\n";

    // lista enlazada local para recordar a los marcados y restaurarlos al final
    struct MarkNode {
        MafiaNode* member;
        MarkNode* next;
    };
    MarkNode* marked = nullptr;

    // el jefe actual queda fuera del puesto para proyectar la cadena
    if (!boss->is_dead) {
        boss->is_dead = true;
        marked = new MarkNode{boss, marked};
    }

    MafiaNode* current = boss;
    int order = 0;
    while (true) {
        MafiaNode* heir = findSuccessor(current, false);
        if (heir == nullptr) {
            heir = findSuccessor(current, true);  // ultimo recurso: presos vivos
        }
        if (heir == nullptr) {
            break;
        }
        order++;
        std::cout << "  " << order << ". ";
        printMemberLine(heir);
        // el heredero listado queda fuera de juego para hallar al siguiente,
        // y la proxima busqueda parte desde su posicion (el ahora seria el jefe)
        heir->is_dead = true;
        marked = new MarkNode{heir, marked};
        current = heir;
    }

    if (order == 0) {
        std::cout << "  (nadie puede tomar el puesto)\n";
    }

    // restaurar el estado real: todos los marcados estaban vivos antes de simular
    while (marked != nullptr) {
        MarkNode* toDelete = marked;
        marked->member->is_dead = false;
        marked = marked->next;
        delete toDelete;
    }
}

// primer miembro elegible en preorden dentro del subarbol (incluye a node): sigue
// la linea del primer sucesor (rama izquierda) hasta el fondo antes de pasar a la
// otra rama. asi el nieto por la rama del primer sucesor hereda antes que el
// segundo sucesor directo (aclaracion del profesor).
MafiaFamily::MafiaNode* MafiaFamily::findEligiblePreorder(MafiaNode* node, bool allowJailed) const {
    if (node == nullptr) {
        return nullptr;
    }
    if (isEligible(node, allowJailed)) {
        return node;
    }
    MafiaNode* found = findEligiblePreorder(node->left, allowJailed);
    if (found != nullptr) {
        return found;
    }
    return findEligiblePreorder(node->right, allowJailed);
}

// busca un sucesor en el subarbol de un jefe siguiendo la linea del primer sucesor
// en profundidad. excluye la raiz (el jefe que deja el puesto): arranca por su
// primer subordinado y recorre esa rama entera antes de pasar al segundo.
MafiaFamily::MafiaNode* MafiaFamily::findHeirInSubtree(MafiaNode* subRoot, bool allowJailed) const {
    if (subRoot == nullptr) {
        return nullptr;
    }
    MafiaNode* found = findEligiblePreorder(subRoot->left, allowJailed);
    if (found != nullptr) {
        return found;
    }
    return findEligiblePreorder(subRoot->right, allowJailed);
}

// primer miembro elegible de todo el arbol siguiendo la linea del primer sucesor
// desde la cabeza de la familia (preorden desde la raiz). ultimo recurso.
MafiaFamily::MafiaNode* MafiaFamily::findAnyEligible(bool allowJailed) const {
    return findEligiblePreorder(root, allowJailed);
}

// primer jefe a la profundidad indicada que tenga sus dos subordinados presentes
// y fuera de la carcel; devuelve al primero de esos dos que sea elegible.
MafiaFamily::MafiaNode* MafiaFamily::findTwoFreeAtDepth(MafiaNode* node, int depth,
                                                        bool allowJailed) const {
    if (node == nullptr) {
        return nullptr;
    }
    if (depth == 0) {
        MafiaNode* left = node->left;
        MafiaNode* right = node->right;
        // "dos sucesores fuera de la carcel": ambos existen y ninguno esta preso.
        if (left != nullptr && right != nullptr && !left->in_jail && !right->in_jail) {
            if (isEligible(left, allowJailed)) {
                return left;
            }
            if (isEligible(right, allowJailed)) {
                return right;
            }
        }
        return nullptr;
    }
    MafiaNode* found = findTwoFreeAtDepth(node->left, depth - 1, allowJailed);
    if (found != nullptr) {
        return found;
    }
    return findTwoFreeAtDepth(node->right, depth - 1, allowJailed);
}

// recorre el arbol generacion por generacion desde la raiz y devuelve al primer
// sucesor elegible del jefe mas cercano a la cabeza de la familia que tenga dos
// subordinados libres.
MafiaFamily::MafiaNode* MafiaFamily::findBossWithTwoFreeSuccessors(bool allowJailed) const {
    if (root == nullptr) {
        return nullptr;
    }
    int height = treeHeight(root);
    for (int depth = 0; depth < height; ++depth) {
        MafiaNode* found = findTwoFreeAtDepth(root, depth, allowJailed);
        if (found != nullptr) {
            return found;
        }
    }
    return nullptr;
}

// Aplica las reglas de sucesion (ADR-003) para hallar el reemplazo de un jefe
// que deja el puesto. La busqueda escala nivel por nivel: nunca salta a la raiz.
MafiaFamily::MafiaNode* MafiaFamily::findSuccessor(MafiaNode* leavingBoss, bool allowJailed) const {
    if (leavingBoss == nullptr) {
        return nullptr;
    }

    // Regla 1: buscar en el propio subarbol del jefe saliente.
    MafiaNode* heir = findHeirInSubtree(leavingBoss, allowJailed);
    if (heir != nullptr) {
        return heir;
    }

    // Reglas 2-3: escalar nivel por nivel mirando la rama del sucesor hermano.
    MafiaNode* current = leavingBoss;
    MafiaNode* parent = findById(root, current->id_boss);
    while (parent != nullptr) {
        MafiaNode* sibling = (parent->left == current) ? parent->right : parent->left;
        if (sibling != nullptr) {
            if (isEligible(sibling, allowJailed)) {
                return sibling;                       // el hermano mismo se vuelve jefe
            }
            heir = findHeirInSubtree(sibling, allowJailed);
            if (heir != nullptr) {
                return heir;
            }
        }
        current = parent;
        parent = findById(root, current->id_boss);
    }

    // regla 5: agotadas las ramas hermanas, tomar al primer sucesor elegible del
    // jefe mas cercano a la cabeza que tenga dos subordinados libres.
    heir = findBossWithTwoFreeSuccessors(allowJailed);
    if (heir != nullptr) {
        return heir;
    }

    // ultimo recurso: cualquier miembro elegible del arbol (el mas cercano a la raiz).
    return findAnyEligible(allowJailed);
}

// Reasigna el jefe si el actual ya no puede ejercer (muerto, preso o mayor de 70).
void MafiaFamily::reassignBossIfNeeded() {
    MafiaNode* boss = findCurrentBoss(root);
    if (boss == nullptr) {
        // Caso borde: no hay jefe marcado; se elige al primer elegible del arbol.
        MafiaNode* candidate = findAnyEligible(false);
        if (candidate == nullptr) {
            candidate = findAnyEligible(true);
        }
        if (candidate != nullptr) {
            candidate->is_boss = true;
            std::cout << "Se asigno un jefe inicial: ";
            printMemberLine(candidate);
        }
        return;
    }

    if (isEligible(boss)) {
        return;  // el jefe sigue siendo apto: no hay nada que reasignar
    }

    // El jefe dejo el puesto. Primero se busca un sucesor libre.
    MafiaNode* successor = findSuccessor(boss, false);
    if (successor == nullptr) {
        // Regla 5: no quedan libres; se admite a los presos que sigan vivos.
        successor = findSuccessor(boss, true);
    }

    boss->is_boss = false;
    boss->was_boss = true;

    if (successor == nullptr) {
        std::cout << "No queda ningun sucesor posible: la familia se quedo sin jefe.\n";
        return;
    }

    successor->is_boss = true;
    std::cout << "El puesto cambio de manos. Nuevo jefe: ";
    printMemberLine(successor);
}

// Imprime todos los miembros del arbol en preorden.
void MafiaFamily::printAll(MafiaNode* node) const {
    if (node == nullptr) {
        return;
    }
    std::cout << "  ";
    printMemberLine(node);
    printAll(node->left);
    printAll(node->right);
}

// Lista todos los miembros (util para que el usuario identifique un id a editar).
void MafiaFamily::listAllMembers() const {
    std::cout << "=== Miembros de la familia ===\n";
    if (root == nullptr) {
        std::cout << "  (no hay miembros cargados)\n";
        return;
    }
    printAll(root);
}

// Edita los datos de un miembro. No se permite modificar id ni id_boss, ya que
// definen la identidad y la posicion del nodo dentro del arbol.
bool MafiaFamily::editMember(int id) {
    MafiaNode* node = findById(root, id);
    if (node == nullptr) {
        std::cout << "No existe un miembro con id " << id << ".\n";
        return false;
    }

    std::cout << "Editando a ";
    printMemberLine(node);
    std::cout << "Campo a modificar:\n"
              << "  1) Nombre\n"
              << "  2) Apellido\n"
              << "  3) Genero (H/M)\n"
              << "  4) Edad\n"
              << "  5) Vivo / Muerto\n"
              << "  6) Libre / Preso\n"
              << "  7) Fue jefe (was_boss)\n"
              << "  8) Es jefe (is_boss)\n"
              << "  0) Cancelar\n> ";

    int option;
    if (!(std::cin >> option)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Entrada invalida.\n";
        return false;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (option) {
        case 1: {
            std::cout << "Nuevo nombre: ";
            std::string value;
            std::getline(std::cin, value);
            if (!value.empty()) {
                node->name = value;
            }
            break;
        }
        case 2: {
            std::cout << "Nuevo apellido: ";
            std::string value;
            std::getline(std::cin, value);
            if (!value.empty()) {
                node->last_name = value;
            }
            break;
        }
        case 3: {
            std::cout << "Nuevo genero (H/M): ";
            std::string value;
            std::getline(std::cin, value);
            if (!value.empty() && (value[0] == 'H' || value[0] == 'M')) {
                node->gender = value[0];
            } else {
                std::cout << "Genero invalido; se mantiene el actual.\n";
            }
            break;
        }
        case 4: {
            std::cout << "Nueva edad: ";
            int value;
            if (std::cin >> value && value >= 0) {
                node->age = value;
            } else {
                std::cin.clear();
                std::cout << "Edad invalida; se mantiene la actual.\n";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 5: {
            std::cout << "Estado (0 = vivo, 1 = muerto): ";
            int value;
            if (std::cin >> value && (value == 0 || value == 1)) {
                node->is_dead = (value == 1);
            } else {
                std::cin.clear();
                std::cout << "Valor invalido; se mantiene el actual.\n";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 6: {
            std::cout << "Estado (0 = libre, 1 = preso): ";
            int value;
            if (std::cin >> value && (value == 0 || value == 1)) {
                node->in_jail = (value == 1);
            } else {
                std::cin.clear();
                std::cout << "Valor invalido; se mantiene el actual.\n";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 7: {
            std::cout << "Fue jefe (0 = no, 1 = si): ";
            int value;
            if (std::cin >> value && (value == 0 || value == 1)) {
                node->was_boss = (value == 1);
            } else {
                std::cin.clear();
                std::cout << "Valor invalido; se mantiene el actual.\n";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 8: {
            std::cout << "Es jefe (0 = no, 1 = si): ";
            int value;
            if (std::cin >> value && (value == 0 || value == 1)) {
                // se normaliza la invariante de jefe unico: al nombrar un jefe
                // nuevo, el anterior deja el puesto y queda como ex-jefe.
                if (value == 1 && !node->is_boss) {
                    MafiaNode* previousBoss = findCurrentBoss(root);
                    if (previousBoss != nullptr && previousBoss != node) {
                        previousBoss->is_boss = false;
                        previousBoss->was_boss = true;
                        std::cout << "El puesto le fue retirado a "
                                  << previousBoss->name << " " << previousBoss->last_name << ".\n";
                    }
                    node->is_boss = true;
                } else if (value == 0 && node->is_boss) {
                    // al quitarle el puesto a mano, este pasa a su sucesor segun
                    // las reglas (si quedara sin jefe, el propio ex-jefe seria
                    // reelegido por la reasignacion automatica al seguir apto).
                    node->is_boss = false;
                    node->was_boss = true;
                    MafiaNode* successor = findSuccessor(node, false);
                    if (successor == nullptr) {
                        successor = findSuccessor(node, true);
                    }
                    if (successor != nullptr) {
                        successor->is_boss = true;
                        std::cout << "El puesto pasa a " << successor->name
                                  << " " << successor->last_name << ".\n";
                    }
                }
            } else {
                std::cin.clear();
                std::cout << "Valor invalido; se mantiene el actual.\n";
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 0:
            std::cout << "Edicion cancelada.\n";
            return false;
        default:
            std::cout << "Opcion no reconocida.\n";
            return false;
    }

    std::cout << "Datos actualizados: ";
    printMemberLine(node);
    return true;
}

// Bucle interactivo principal. Se ejecuta hasta que el usuario elige salir (0).
void MafiaFamily::run() {
    int option = -1;
    do {
        std::cout << "\n===== CASA NOSTRA - Sistema de sucesion =====\n"
                  << "  1) Ver linea de sucesion actual\n"
                  << "  2) Listar todos los miembros\n"
                  << "  3) Editar un miembro\n"
                  << "  0) Salir\n> ";

        if (!(std::cin >> option)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrada invalida. Intente de nuevo.\n";
            option = -1;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (option) {
            case 1:
                reassignBossIfNeeded();   // asegura que el jefe vigente sea apto
                showSuccessionLine();
                break;
            case 2:
                listAllMembers();
                break;
            case 3: {
                std::cout << "Id del miembro a editar: ";
                int id;
                if (std::cin >> id) {
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    editMember(id);
                    reassignBossIfNeeded();  // el cambio puede dejar sin puesto al jefe
                } else {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Id invalido.\n";
                }
                break;
            }
            case 0:
                std::cout << "Cerrando el sistema. Que la familia perdure.\n";
                break;
            default:
                std::cout << "Opcion no reconocida.\n";
                break;
        }
    } while (option != 0);
}
