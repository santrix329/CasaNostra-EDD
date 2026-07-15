#include <iostream>
#include "mafia_family.h"

int main() {
    MafiaFamily family;

    if (!family.loadFromCsv("datos_familia.csv")) {
        return 1;
    }

    std::cout << "Familia cargada: " << family.countMembers() << " miembros." << std::endl;
    return 0;
}
