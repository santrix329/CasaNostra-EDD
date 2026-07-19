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
│   ├── datos_familia.csv     # datos de prueba
│   └── mafia_app.exe         # ejecutable generado
├── src/
│   ├── main.cpp              # punto de entrada
│   ├── mafia_family.h        # clase MafiaFamily (interfaz) + struct MafiaNode anidado
│   └── mafia_family.cpp      # implementación de la clase
└── README.md
```

## Diseño

- **`MafiaFamily`**: clase de dominio que representa a la familia. Encapsula la
  raíz del árbol y toda la lógica, y gestiona su propia memoria mediante un
  destructor recursivo (RAII).
- **`MafiaNode`**: `struct` interno y privado de `MafiaFamily`. Guarda los datos
  de un miembro y los punteros a sus dos subordinados (`left`, `right`). Solo se
  accede a él a través de los métodos de la clase.
- El árbol se ordena **por jefatura** (`id_boss`), no por comparación de valores:
  cada miembro cuelga de su jefe directo.
- **No se utilizan vectores.** Las estructuras auxiliares (carga del CSV,
  proyección de la sucesión) se resuelven con listas enlazadas y punteros.

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
- La carga **no depende del orden** de las filas: un subordinado puede aparecer
  antes que su jefe y el árbol se arma igual.

## Funcionalidades

1. **Carga automática** del árbol desde el CSV según la relación de jefatura.
2. **Línea de sucesión actual**: proyecta, en orden, la cadena de personas que
   tomarían el puesto aplicando las condiciones de sucesión sobre el estado
   actual de los miembros (solo vivos; los presos aparecen únicamente cuando ya
   no quedan candidatos libres).
3. **Reasignación automática del jefe** cuando el actual muere, va preso o supera
   el límite de edad (70 años), siguiendo la escalada de reglas de sucesión.
4. **Edición** de los datos de cualquier miembro, excepto `id` e `id_boss`.
5. El programa se mantiene en ejecución hasta que el usuario elige salir.

## Reglas de sucesión

Cuando el jefe deja el puesto, el reemplazo se busca en este orden:

1. Se sigue la **línea del primer sucesor** en profundidad, hasta hallar a alguien
   vivo y fuera de la cárcel.
2. Si su árbol no da a nadie, se escala **un nivel** y se busca en la rama del
   sucesor compañero; y así sucesivamente hacia arriba.
3. Agotadas las ramas, se toma al primer sucesor elegible del **jefe más cercano
   con dos subordinados libres**.
4. Si ya no quedan candidatos libres, se repite la búsqueda admitiendo a los
   **presos que sigan vivos**.

En todos los casos se descarta a los muertos, y a los mayores de 70 años como
sucesores.

## Compilación

Desde la raíz del proyecto:

```
g++ -Wall -std=c++17 src/main.cpp src/mafia_family.cpp -o bin/mafia_app.exe
```

## Ejecución

El ejecutable y el CSV conviven en `bin/`. Se puede correr desde la raíz del
proyecto o desde `bin/` (doble clic al `.exe`); el programa resuelve la ruta del
CSV en ambos casos.

```
./bin/mafia_app.exe
```

## Librerías utilizadas

El proyecto usa **únicamente la biblioteca estándar de C++** (sin dependencias
externas ni contenedores prohibidos):

| Cabecera | Uso |
|----------|-----|
| `<iostream>` | Entrada y salida por consola |
| `<string>` | Manejo de cadenas (`std::string`) |
| `<fstream>` | Lectura del archivo CSV |
| `<sstream>` | Separación de los campos de cada línea |
| `<iomanip>` | Alineación de las columnas en pantalla |
| `<limits>` | Limpieza del buffer de entrada del usuario |

## Requisitos

- Compilador con soporte de C++17 (por ejemplo, g++).
