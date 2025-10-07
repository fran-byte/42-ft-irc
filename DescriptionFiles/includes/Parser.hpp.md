`Parser.hpp` :

---

### Estructura y función principal

#### 1. Inclusión de cabeceras

```c++
#include <vector>
#include <string>
#include <sstream>
```

Estas cabeceras permiten usar:

- **vector:** Para almacenar listas dinámicas (en este caso, los argumentos del comando).
- **string:** Para manipulación de cadenas de texto.
- **sstream:** Para leer cadenas como si fueran flujos (útil para separar palabras).

---

#### 2. Estructura Cmd

```c++
struct Cmd {
    std::string verb;
    std::vector<std::string> args;
};
```

Define una estructura llamada `Cmd` que representa un comando IRC:

- `verb`: el verbo del comando (por ejemplo, "PRIVMSG", "JOIN", etc.).
- `args`: los argumentos del comando, almacenados como un vector de strings.

---

#### 3. Función parseIrcLine

```c++
inline Cmd parseIrcLine(const std::string& line) {
    Cmd out;
    std::string s = line, trailing;
    std::string::size_type p = s.find(" :");
    if (p != std::string::npos) {
        trailing = s.substr(p + 2);
        s.erase(p);
    }
    std::istringstream iss(s);
    iss >> out.verb;
    for (std::string tok; iss >> tok; )
        out.args.push_back(tok);
    if (!trailing.empty())
        out.args.push_back(trailing);
    for (size_t i=0;i<out.verb.size();++i)
        out.verb[i]=std::toupper(out.verb[i]);
    return out;
}
```

##### Paso a paso

1. **Inicialización:**

   - Se crea una variable `out` de tipo `Cmd`.
   - Se copia la línea de entrada a la variable `s`.
   - Se declara una variable `trailing` (parte final del mensaje, que puede contener espacios y empieza después de `" :"`).

2. **Búsqueda de " :"**

   - En IRC, los parámetros después de `" :"` se consideran un solo argumento aunque contengan espacios (por ejemplo: `PRIVMSG #canal :Hola mundo`).
   - Se busca `" :”` en la cadena. Si existe:
     - `trailing` toma el valor de todo lo que está después de `" :"`.
     - Se borra esa parte de `s` para que solo contenga los argumentos "normales".

3. **Separación del verbo y argumentos:**

   - Se crea un flujo de entrada con la cadena `s`.
   - Se extrae la primera palabra a `out.verb` (el comando principal).
   - Se extraen el resto de palabras a `out.args` (los argumentos).

4. **Agregado del argumento final (`trailing`):**

   - Si existe `trailing`, se añade como último argumento. Así se preservan los espacios que puede tener este campo.

5. **Conversión del verbo a mayúsculas:**

   - Se recorre cada carácter de `out.verb` y se convierte a mayúscula (IRC no distingue entre mayúsculas/minúsculas en el verbo del comando).

6. **Retorno:**
   - Devuelve la estructura `Cmd` con el verbo y los argumentos ya separados.

---

### Ejemplo de uso

Si la línea es:

```
PRIVMSG #canal :Hola mundo a todos
```

- `out.verb` será `"PRIVMSG"`
- `out.args` tendrá dos elementos:
  1. `"#canal"`
  2. `"Hola mundo a todos"`

---

### Resumen

Esta función toma una línea de comando IRC, separa el verbo y los argumentos (incluyendo correctamente la parte final que puede contener espacios), y los devuelve en una estructura fácil de usar para el resto del programa. Además, normaliza el verbo a mayúsculas para facilitar comparaciones.
