- `Topic.cpp` en concreto de la función `Server::cmdTOPIC`:

---

## ¿Qué hace este código?

Implementa el comando IRC `/TOPIC` en un servidor, permitiendo a los clientes ver o cambiar el "topic" (tema) de un canal.

---

## Desglose línea por línea

```c++
#include "Server.hpp"
```

Incluye la definición de la clase `Server` y sus métodos (y probablemente también las clases `Client`, `Channel`, etc.).

---

```c++
void Server::cmdTOPIC(Client& c, const std::vector<std::string>& a){
```

Define el método `cmdTOPIC` dentro de la clase `Server`.

- **`Client& c`**: el cliente que envió el comando.
- **`const std::vector<std::string>& a`**: los argumentos del comando (por ejemplo: `#canal`, o `#canal :nuevo topic`).

---

### 1. Validar argumentos

```c++
if (a.empty()) return sendTo(c, ":server NOTICE " + c.nick + " :TOPIC needs #channel\r\n");
```

- Si no se pasan argumentos, envía un mensaje de error al cliente diciendo que falta el canal.

---

### 2. Buscar el canal

```c++
const std::string& chan = a[0];
std::map<std::string,Channel>::iterator it = channels.find(chan);
if (it == channels.end()) return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");
```

- Extrae el nombre del canal del primer argumento.
- Busca el canal en el mapa de canales (`channels`).
- Si el canal no existe, devuelve un error al cliente.

---

### 3. Mostrar el topic (si no se da uno nuevo)

```c++
Channel& ch = it->second;
// Mostrar topic si no hay texto
if (a.size() == 1) {
    std::string t = ch.topic.empty()? "No topic is set" : ch.topic;
    return sendTo(c, ":server NOTICE " + c.nick + " :" + chan + " : " + t + "\r\n");
}
```

- Si solo se pasa el canal como argumento (`/TOPIC #canal`), muestra el topic actual.
- Si el topic está vacío, muestra "No topic is set".

---

### 4. Cambiar el topic (si se da un nuevo topic)

```c++
// Cambiar topic: validar permisos
if (ch.topicOpOnly && !ch.isOp(c.fd))
    return sendTo(c, ":server NOTICE " + c.nick + " :You're not channel operator\r\n");

ch.topic = a[1]; // llega como trailing ya
std::string msg = ":" + c.nick + " TOPIC " + chan + " :" + ch.topic + "\r\n";
broadcastToChannel(ch, -1, msg);
```

- Si el canal está configurado para que solo los operadores puedan cambiar el topic (`ch.topicOpOnly`), verifica que el cliente sea operador (`ch.isOp(c.fd)`).
  - Si no lo es, manda un mensaje de error.
- Si pasa la verificación, cambia el topic del canal al nuevo valor (`a[1]`).
- Construye el mensaje IRC con el nuevo topic y lo envía a todos los usuarios del canal (`broadcastToChannel`).

---

## Resumen de flujo

1. Si faltan argumentos → error.
2. Si el canal no existe → error.
3. Si solo se pide ver el topic → se muestra.
4. Si se quiere cambiar el topic:
   - Si solo operadores pueden y el usuario no lo es → error.
   - Si puede, cambia el topic y lo anuncia a todos.

---

## ¿Qué funciones auxiliares hay?

- **sendTo(c, mensaje):** Envía un mensaje a un cliente específico.
- **broadcastToChannel(ch, -1, mensaje):** Envía un mensaje a todos los clientes de un canal (probablemente excepto al fd -1, que significa "a todos").
- **isOp(fd):** Devuelve true si el cliente con ese file descriptor es operador en el canal.

---
