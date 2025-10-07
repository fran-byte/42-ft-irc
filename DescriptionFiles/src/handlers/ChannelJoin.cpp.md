- `ChannelJoin.cpp` funcionamiento de los métodos `cmdJOIN` y `cmdPART` en el contexto de un servidor IRC:

---

## Contexto general

El archivo define dos métodos de la clase `Server`:

- `cmdJOIN`: Maneja la lógica cuando un cliente quiere unirse a un canal.
- `cmdPART`: Maneja la lógica cuando un cliente quiere salir de un canal.

Estos métodos procesan los comandos IRC `JOIN` y `PART` enviados por los usuarios.

---

## Método: `void Server::cmdJOIN(Client& c, const std::vector<std::string>& a)`

### 1. Validaciones iniciales

- **Verifica si se especificó un canal:**

  ```c++
  if (a.empty())
      return sendTo(c, ":server NOTICE " + (c.nick.empty()?"*":c.nick) + " :JOIN needs channel\r\n");
  ```

  Si el vector de argumentos está vacío, informa al usuario que debe indicar el canal.

- **Verifica el formato del nombre del canal:**
  ```c++
  std::string chan = a[0];
  if (chan.empty() || chan[0] != '#')
      return sendTo(c, ":server NOTICE " + c.nick + " :Bad channel name\r\n");
  ```
  El nombre del canal debe empezar con `#`. Si no, devuelve un mensaje de error.

### 2. Manejo de clave de canal (opcional)

- **Clave opcional para canales protegidos:**
  ```c++
  std::string *keyOpt = 0;
  std::string keyTmp;
  if (a.size() >= 2) { keyTmp = a[1]; keyOpt = &keyTmp; }
  ```
  Si se pasa un segundo argumento, lo considera como la clave del canal.

### 3. Obtención o creación del canal

- **Obtiene o crea el canal automáticamente:**
  ```c++
  Channel &ch = channels[chan]; // crea si no existe
  if (ch.name.empty()) ch.name = chan;
  bool newlyCreated = ch.members.empty();
  ```
  Si el canal no existe en el mapa, lo crea. Si no tiene miembros, marca que es nuevo.

### 4. Validación para unirse al canal

- **Chequea permisos y clave:**
  ```c++
  if (!ch.canJoin(c.fd, keyOpt))
      return sendTo(c, ":server NOTICE " + c.nick + " :Cannot join channel\r\n");
  ```
  Usa el método `canJoin` del canal para comprobar si el usuario puede entrar (por ejemplo, clave correcta, límites, etc).

### 5. Añade al miembro y notifica

- **Agrega al usuario como miembro:**

  ```c++
  ch.addMember(c.fd);
  c.channels.insert(chan);
  if (newlyCreated) ch.addOp(c.fd);
  ```

  Añade el miembro al canal y, si el canal es nuevo, lo hace operador.

- **Construye y envía el mensaje JOIN:**
  ```c++
  std::string joinMsg = ":" + (c.nick.empty()? "*": c.nick) + " JOIN " + chan + "\r\n";
  broadcastToChannel(ch, -1, joinMsg); // a todos
  ```
  Informa a todos los miembros del canal que el usuario se unió.

---

## Método: `void Server::cmdPART(Client& c, const std::vector<std::string>& a)`

### 1. Validaciones iniciales

- **Verifica si se especificó el canal:**

  ```c++
  if (a.empty())
      return sendTo(c, ":server NOTICE " + (c.nick.empty()?"*":c.nick) + " :PART needs channel\r\n");
  ```

- **Busca el canal en el mapa:**

  ```c++
  std::string chan = a[0];
  std::map<std::string, Channel>::iterator it = channels.find(chan);
  if (it == channels.end())
      return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");
  ```

- **Verifica que el usuario sea miembro:**
  ```c++
  Channel &ch = it->second;
  if (!ch.isMember(c.fd))
      return sendTo(c, ":server NOTICE " + c.nick + " :You're not on that channel\r\n");
  ```

### 2. Notifica la salida y actualiza estructuras

- **Construye y envía el mensaje PART:**

  ```c++
  std::string partMsg = ":" + c.nick + " PART " + chan + "\r\n";
  broadcastToChannel(ch, c.fd, partMsg); // a otros miembros
  sendTo(c, partMsg);                    // al propio usuario
  ```

- **Elimina al usuario del canal y viceversa:**
  ```c++
  ch.removeMember(c.fd);
  c.channels.erase(chan);
  if (ch.members.empty()) channels.erase(it);
  ```
  Si el canal queda vacío, se elimina del mapa.

---

## Resumen gráfico

```text
JOIN
 └─> Valida argumentos
 └─> Obtiene/crea canal
 └─> Chequea permisos
 └─> Añade miembro
 └─> Notifica a todos

PART
 └─> Valida argumentos
 └─> Busca canal y verifica membresía
 └─> Notifica salida
 └─> Elimina usuario y canal si vacío
```

---

## Conclusión

- Ambos métodos gestionan la lógica principal de entrada y salida de canales en un servidor IRC.
- Aseguran las validaciones necesarias y mantienen la consistencia interna de los miembros y canales.
- Se encargan de notificar a los usuarios implicados según el protocolo IRC.
