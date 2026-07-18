#include <iostream>
#include "mafia_family.h"

int main() {
    MafiaFamily family;

    if (!family.loadFromCsv("bin/datos_familia.csv")) {
        // Si falla, intenta cargar si se ejecuta directamente desde la carpeta bin (doble clic al .exe)
        if (!family.loadFromCsv("datos_familia.csv")) {
            std::cerr << "Error critico: No se encontro datos_familia.csv en ninguna ruta.\n";
            std::cin.get(); // Pausa para que veas el error antes de cerrarse
            return 1;
        }
    }

    family.reassignBossIfNeeded();  // por si el CSV trae un jefe que ya no puede ejercer
    family.run();

    // Pausa final para que la ventana no se cierre de golpe al elegir Opcion 0 (Salir)
    std::cout << "\nPresiona Enter para salir...";
    std::cin.clear();
    std::cin.get(); 
    
    return 0;
}