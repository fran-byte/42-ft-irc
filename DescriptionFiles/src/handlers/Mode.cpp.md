- `Mode.cpp` (handler del comando IRC `MODE`) línea por línea y por bloques funcionales:

---

## **1. Inclusiones y función auxiliar**

```c++
#include "Server.hpp"
#include <cstdlib>

static bool needsArg(char f){ return f=='k' || f=='o' || f=='l'; }
```

- **Incluye** el header del servidor y `<cstdlib>` para usar funciones como `atoi`.
- **Función auxiliar** `needsArg`: Devuelve `true` si el modo (`f`) requiere argumento (k=key, o=operator, l=limit).

---

## **2. Función principal: `Server::cmdMODE`**

```c++
void Server::cmdMODE(Client& c, const std::vector<std::string>& a){
```

- Define el handler del comando `MODE`.
- `c` es el cliente que envía el comando,
- `a` es el vector de argumentos (a[0]=target, a[1]=modos, a[2..]=args).

---

### **a. Comprobaciones iniciales**

```c++
    if (a.empty()) return sendTo(c, ":server NOTICE " + c.nick + " :MODE needs a target\r\n");
```

- Si no hay argumentos, avisa que falta el target (canal o usuario).

```c++
    const std::string& target = a[0];
    if (target.empty() || target[0] != '#')
        return sendTo(c, ":server NOTICE " + c.nick + " :Only channel modes supported\r\n");
```

- El target debe empezar por `#` (solo soporta canales).

```c++
    std::map<std::string,Channel>::iterator it = channels.find(target);
    if (it == channels.end()) return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");
    Channel& ch = it->second;
```

- Busca el canal en el mapa `channels`.
- Si no existe, avisa.
- Si existe, referencia al canal como `ch`.

---

### **b. Mostrar modos actuales del canal (consulta)**

```c++
    if (a.size()==1) {
        // Mostrar modos actuales (simple)
        std::string modes = "+";
        if (ch.inviteOnly)  modes += "i";
        if (ch.topicOpOnly) modes += "t";
        if (!ch.key.empty())modes += "k";
        if (ch.limit >= 0)  modes += "l";
        return sendTo(c, ":server NOTICE " + c.nick + " :" + target + " " + modes + "\r\n");
    }
```

- Si solo se especifica el canal (no modos), muestra los modos activos:
  - `i`: Solo con invitación.
  - `t`: Solo operadores pueden cambiar topic.
  - `k`: Hay clave puesta.
  - `l`: Hay límite de usuarios.

---

### **c. Comprobación de permisos**

```c++
    if (!ch.isOp(c.fd)) return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");
```

- Solo los operadores del canal pueden cambiar modos.

---

### **d. Procesar modos y argumentos**

```c++
    std::string modeStr = a[1];
    std::vector<std::string> args;
    for (size_t i=2;i<a.size();++i) args.push_back(a[i]);
```

- `modeStr` contiene la cadena de modos (ej: `+it-k`).
- `args` contiene los argumentos extra.

---

#### **Iterar y aplicar modos**

```c++
    bool set = true;
    for (size_t i=0;i<modeStr.size();++i){
        char m = modeStr[i];
        if (m == '+'){ set = true;  continue; }
        if (m == '-'){ set = false; continue; }

        std::string arg;
        if (needsArg(m) && set){
            if (args.empty()) continue; // falta arg -> lo ignoramos
            arg = args.front(); args.erase(args.begin());
        }

        switch(m){
            case 'i': ch.setInviteOnly(set); break;
            case 't': ch.setTopicRestricted(set); break;
            case 'k':
                if (set) ch.setKey(arg); else ch.clearKey();
                break;
            case 'l':
                if (set) ch.setLimit(std::atoi(arg.c_str())); else ch.clearLimit();
                break;
            case 'o': {
                int fdTarget = findFdByNick(arg);
                if (fdTarget<0 || !ch.isMember(fdTarget)) break;
                if (set) ch.ops.insert(fdTarget); else ch.ops.erase(fdTarget);
            } break;
            default: /* ignorar */ break;
        }
    }
```

- Itera la cadena de modos:
  - `+` o `-` cambian la acción (set/unset).
  - Si el modo necesita argumento (`k`, `o`, `l`), consume un argumento de la lista.
- `switch`:
  - `i`: Modo invite-only.
  - `t`: Solo operadores pueden cambiar topic.
  - `k`: Clave del canal.
  - `l`: Límite de usuarios.
  - `o`: Dar o quitar operator a alguien (`arg` es el nick). Busca su file descriptor y lo añade o elimina de los operadores.
  - Otros caracteres: ignorados.

---

### **e. Notificación al canal**

```c++
    // Anunciar (simple)
    std::string notif = ":" + c.nick + " MODE " + target + " " + modeStr;
    for (size_t i=0;i<args.size();++i) notif += " " + args[i]; // (solo si quieres reflejar)
    notif += "\r\n";
    broadcastToChannel(ch, -1, notif);
}
```

- Construye un mensaje de notificación del cambio de modos.
- Lo envía a todos los usuarios del canal (`broadcastToChannel`).

---

## **Resumen funcional**

- **Consulta modos** de un canal: `/MODE #canal`
- **Cambia modos** de un canal: `/MODE #canal +it -k` (requiere ser operador)
- **Modos soportados:**
  - `i`: invite-only
  - `t`: topic solo op
  - `k`: clave
  - `l`: límite de usuarios
  - `o`: (des)op
- **Controla argumentos y permisos.**
- **Notifica** los cambios a todos los usuarios del canal.
