#include <iostream>
#include "mafia_family.h"

int main() {
    MafiaFamily family;

    if (!family.loadFromCsv("datos_familia.csv")) {
        return 1;
    }

    family.reassignBossIfNeeded();  // por si el CSV trae un jefe que ya no puede ejercer
    family.run();
    return 0;
}
