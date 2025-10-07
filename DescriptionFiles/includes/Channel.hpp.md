- `Channel.hpp` define una estructura (`struct`) en C++ llamada `Channel`, que representa un canal en un servidor IRC (Internet Relay Chat).

---

### 1. **Cabeceras y dependencias**

```c++
#pragma once
#include <string>
#include <set>
```

- `#pragma once`: Indica al compilador que solo incluya este archivo una vez por compilación, evitando dobles definiciones.
- `<string>`: Sirve para usar la clase `std::string`.
- `<set>`: Permite usar el contenedor `std::set`, que almacena elementos únicos y ordenados.

---

### 2. **Definición de la estructura Channel**

```c++
struct Channel {
    std::string name, topic, key;
    bool inviteOnly;     // +i
    bool topicOpOnly;    // +t
    int  limit;          // +l; -1 = sin límite
    std::set<int> members, ops, invited;
    ...
};
```

- **Atributos principales:**
  - `name`: Nombre del canal (ej: `#general`).
  - `topic`: Tema o descripción del canal.
  - `key`: Clave de acceso (si el canal está protegido con contraseña, modo +k).
  - `inviteOnly`: Si es `true`, solo usuarios invitados pueden entrar (+i).
  - `topicOpOnly`: Si es `true`, solo los operadores pueden cambiar el tema (+t).
  - `limit`: Límite de usuarios permitidos (+l). Si es `-1`, no hay límite.
  - `members`: Conjunto de identificadores de usuarios (fd = file descriptor, normalmente un entero que representa la conexión del usuario) que son miembros del canal.
  - `ops`: Conjunto de operadores del canal (usuarios con privilegios).
  - `invited`: Usuarios invitados al canal.

---

### 3. **Constructores**

```c++
Channel() : inviteOnly(false), topicOpOnly(false), limit(-1) {}
explicit Channel(const std::string& n)
    : name(n), inviteOnly(false), topicOpOnly(false), limit(-1) {}
```

- El primero es el constructor por defecto: crea un canal sin restricciones (no invitación, sin límite, etc.).
- El segundo permite crear un canal con nombre.

---

### 4. **Getters y métodos de comprobación**

```c++
bool isMember(int fd)   const { return members.count(fd)   != 0; }
bool isOp(int fd)       const { return ops.count(fd)       != 0; }
bool isInvited(int fd)  const { return invited.count(fd)   != 0; }
```

- Comprueban si un usuario (identificado por su `fd`) pertenece al canal, es operador o está invitado.

---

### 5. **Setters para modos**

```c++
void setInviteOnly(bool on)      { inviteOnly = on; }
void setTopicRestricted(bool on) { topicOpOnly = on; }
void setKey(const std::string& k){ key = k; }
void clearKey()                  { key.clear(); }
void setLimit(int n)             { limit = n; }
void clearLimit()                { limit = -1; }
```

- Métodos para activar/desactivar modos del canal (+i, +t, +k, +l) o quitar claves/límites.

---

### 6. **Gestión de miembros**

```c++
void addMember(int fd)   { members.insert(fd); invited.erase(fd); }
void removeMember(int fd){ members.erase(fd); ops.erase(fd); invited.erase(fd); }
void addOp(int fd)       { ops.insert(fd); }
void invite(int fd)      { invited.insert(fd); }
```

- `addMember`: Añade un usuario como miembro y elimina su invitación (si tenía).
- `removeMember`: Elimina el usuario del canal, de los operadores y de la lista de invitados.
- `addOp`: Hace operador a un usuario.
- `invite`: Invita a un usuario (lo añade a la lista de invitados).

---

### 7. **Validación de JOIN**

```c++
bool canJoin(int fd, const std::string* providedKey) const {
    if (limit >= 0 && (int)members.size() >= limit) return false;   // +l
    if (!key.empty()) {
        if (!providedKey || *providedKey != key) return false;      // +k
    }
    if (inviteOnly && invited.count(fd) == 0 && members.count(fd) == 0)
        return false;                                               // +i
    return true;
}
```

- Valida si un usuario puede entrar al canal, comprobando:
  1. **Límite de usuarios** (`+l`): si está lleno, no puede entrar.
  2. **Clave** (`+k`): si hay clave, el usuario debe proporcionarla correctamente.
  3. **Solo por invitación** (`+i`): si está el modo activado y el usuario no está invitado ni es miembro, no puede entrar.
  4. Si pasa todas las condiciones, puede entrar.

---

## **Resumen**

La estructura `Channel` encapsula toda la lógica básica de un canal IRC:

- Permite gestionar miembros, operadores, invitados y modos especiales.
- Ofrece funciones para establecer restricciones como límite de usuarios, clave o acceso solo por invitación.
- Facilita la comprobación de permisos y condiciones de entrada.

Esta implementación es sencilla y directa para un servidor IRC, usando identificadores de usuarios como `int` (probablemente los file descriptors de las conexiones de red activas).
