# Casa Nostra — Estructura Dinámica de Datos

Prototipo para la gestión de la sucesión de una familia de la mafia italiana.
El programa lee un archivo `.csv`, construye un **árbol binario de jerarquía**
(cada miembro tiene como máximo dos subordinados) y permite consultar la línea
de sucesión, editar los datos de los miembros y reasignar al jefe de forma
automática cuando el actual ya no puede ejercer.

## Estructura del proyecto

```
CasaNostra-EDD/
├── bin/
│   └── datos_familia.csv      # datos de prueba (y donde se genera el ejecutable)
├── src/
│   ├── main.cpp               # punto de entrada
│   ├── mafia_family.h         # clase MafiaFamily (interfaz) + struct MafiaNode anidado
│   └── mafia_family.cpp       # implementación de la clase
└── README.md
```

## Diseño

- **`MafiaFamily`**: clase de dominio que representa a la familia. Encapsula la
  raíz del árbol y toda la lógica. Gestiona su propia memoria con un destructor
  recursivo (RAII).
- **`MafiaNode`**: `struct` interno y privado de `MafiaFamily`. Guarda los datos
  de un miembro y punteros a sus dos subordinados (`left`, `right`). Solo se
  accede a él a través de los métodos de la clase.
- El árbol se ordena **por jefatura** (`id_boss`), no por comparación de valores:
  cada miembro cuelga de su jefe. No se utilizan vectores.

## Formato del CSV

Cabecera:

```
id,name,last_name,gender,age,id_boss,is_dead,in_jail,was_boss,is_boss
```

- `gender`: `H` (hombre) o `M` (mujer).
- `id_boss`: id del jefe directo; `0` indica la raíz (cabeza de la familia).
- `is_dead`, `in_jail`, `was_boss`, `is_boss`: `1` afirmativo, `0` negativo.
- El primer subordinado que aparece para un jefe ocupa la posición izquierda y
  el segundo la derecha (el orden define la prioridad de sucesión).

## Compilación

Desde la raíz del proyecto:

```
g++ -Wall -std=c++17 src/main.cpp src/mafia_family.cpp -o bin/casa_nostra.exe
```

## Ejecución

El ejecutable y el CSV conviven en `bin/`, así que se ejecuta desde ahí:

```
cd bin
./casa_nostra.exe
```

## Funcionalidades

1. **Carga automática** del árbol desde el CSV según la relación de jefatura.
2. **Línea de sucesión actual**: lista únicamente a los miembros elegibles
   (vivos, libres y dentro del límite de edad), ordenados por generación.
3. **Reasignación automática del jefe** cuando el actual muere, va preso o supera
   el límite de edad, siguiendo la escalada de reglas de sucesión.
4. **Edición** de los datos de cualquier miembro, excepto `id` e `id_boss`.
5. El programa se mantiene en ejecución hasta que el usuario elige salir.
