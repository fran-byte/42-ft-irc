- `Misc.cpp` forma parte del código fuente de un servidor IRC (Internet Relay Chat). El archivo implementa el manejo de algunos comandos básicos que los clientes pueden enviar al servidor.

### 1. Inclusión de dependencias

```c++
#include "Server.hpp"
```

Incluye la definición de la clase `Server` y probablemente otras clases/funciones relacionadas.

---

### 2. Implementación de comandos IRC

#### **Comando PING**

```c++
void Server::cmdPING(Client& c, const std::vector<std::string>& a){
    std::string token = a.empty()? "" : a.back();
    sendTo(c, ":server PONG :"+token+"\r\n");
}
```

**¿Qué hace?**

- Cuando el servidor recibe un comando `PING` de un cliente, responde con un mensaje `PONG`.
- El argumento (`token`) sirve para identificar la petición, y normalmente es enviado de vuelta como confirmación de que el cliente sigue conectado.
- Si no hay argumentos, envía un string vacío como token.

---

#### **Comando QUIT**

```c++
void Server::cmdQUIT(Client& c, const std::vector<std::string>& a) {
    const std::string reason = a.empty() ? "Client Quit" : a.back();
    quitCleanup(c, reason);
    c.closing = true;
}
```

**¿Qué hace?**

- Maneja la desconexión de un cliente.
- El comando `QUIT` puede llevar un motivo (reason) que se envía como argumento. Si no, pone por defecto `"Client Quit"`.
- Llama a una función auxiliar `quitCleanup`, probablemente para limpiar recursos, notificar a otros usuarios, etc.
- Marca el cliente como `closing` (cerrando conexión), lo que seguramente hace que el servidor cierre la conexión más adelante.

---

#### **Manejo General de Comandos**

```c++
void Server::handleCommand(Client& c, const Cmd& cmd){
    if (cmd.verb == "PING") return cmdPING(c, cmd.args);
    if (cmd.verb == "QUIT") return cmdQUIT(c, cmd.args);

    if (!c.registered){
        if (cmd.verb == "PASS") return cmdPASS(c, cmd.args);
        if (cmd.verb == "NICK") return cmdNICK(c, cmd.args);
        if (cmd.verb == "USER") return cmdUSER(c, cmd.args);
        return sendTo(c, ":server NOTICE * :Register first (PASS/NICK/USER)\r\n");
    }

    if (cmd.verb == "JOIN")   return cmdJOIN(c, cmd.args);
    if (cmd.verb == "PART")   return cmdPART(c, cmd.args);
    if (cmd.verb == "PRIVMSG")return cmdPRIVMSG(c, cmd.args);
    if (cmd.verb == "MODE")  return cmdMODE(c, cmd.args);
    if (cmd.verb == "TOPIC") return cmdTOPIC(c, cmd.args);
    if (cmd.verb == "INVITE") return cmdINVITE(c, cmd.args);
    if (cmd.verb == "KICK") return cmdKICK(c, cmd.args);

    sendTo(c, ":server NOTICE " + (c.nick.empty()?"*":c.nick) + " :Unknown command\r\n");
}
```

**¿Qué hace?**

- **Identifica el comando** (`cmd.verb`) y llama a la función correspondiente (por ejemplo, `cmdPING`, `cmdQUIT`).
- **Si el cliente no está registrado** (`!c.registered`):
  - Solo permite comandos de registro (`PASS`, `NICK`, `USER`).
  - Si recibe cualquier otro comando, le avisa que debe registrarse primero.
- **Si el cliente está registrado**:
  - Permite comandos típicos de IRC como `JOIN`, `PART`, `PRIVMSG`, etc.
- **Comando desconocido**:
  - Si el comando no se reconoce, responde con un aviso de "Unknown command".

---

### 3. Resumen de funcionamiento

- El servidor **recibe comandos** de los clientes y los **distribuye** a sus manejadores según el tipo.
- Algunos comandos solo se pueden ejecutar si el cliente está registrado (ya pasó por el proceso de registro IRC).
- Maneja respuestas básicas (`PONG` para `PING`, limpieza y cierre para `QUIT`).
- Si el comando no se reconoce o no está permitido en ese estado, responde con un mensaje de error/aviso.

---

### 4. Notas adicionales

- El código es modular: cada comando tiene su propia función.
- Se utilizan strings para construir los mensajes IRC, siguiendo el protocolo.
- El manejo de registro es importante para la seguridad y el flujo del IRC.
