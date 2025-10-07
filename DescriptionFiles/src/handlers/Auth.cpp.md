- `src/handlers/Auth.cpp` función del proceso de autenticación y registro de un cliente en este servidor IRC:

---

## 1. Contexto General

El archivo implementa **métodos de autenticación y registro** de usuarios en el servidor IRC. El flujo general es:

- El cliente debe enviar correctamente los comandos `PASS`, `NICK` y `USER`.
- Una vez que todos los requisitos están cumplidos, el cliente es marcado como registrado y recibe un mensaje de bienvenida.

---

## 2. Métodos Explicados

### **2.1. `tryRegister(Client& c)`**

```c++
void Server::tryRegister(Client& c){
    if (!c.registered && c.passOk && !c.nick.empty() && !c.user.empty()){
        c.registered = true;
        sendTo(c, ":server 001 " + c.nick + " :Welcome to ft_irc\r\n");
    }
}
```

- **Propósito:** Intenta registrar al cliente `c` si cumple todas las condiciones.
- **Condiciones para registrar:**
  - `!c.registered`: El cliente **no está ya registrado**.
  - `c.passOk`: El cliente **ha pasado la comprobación de contraseña**.
  - `!c.nick.empty()`: El cliente **ha enviado un nickname**.
  - `!c.user.empty()`: El cliente **ha enviado un username**.
- **Acciones al registrar:**
  - Marca el cliente como registrado: `c.registered = true`.
  - Envía mensaje de bienvenida: `:server 001 <nick> :Welcome to ft_irc`.

---

### **2.2. `cmdPASS(Client& c, const std::vector<std::string>& a)`**

```c++
void Server::cmdPASS(Client& c, const std::vector<std::string>& a){
    if (a.empty()) return sendTo(c, ":server NOTICE * :PASS needs parameter\r\n");
    if (c.registered) return;
    c.passOk = (a[0] == password);
    if (!c.passOk) sendTo(c, ":server NOTICE * :Bad password\r\n");
    tryRegister(c);
}
```

- **Propósito:** Procesa el comando `PASS`, que se usa para enviar la contraseña del servidor.
- **Lógica:**
  - Si no se pasa parámetro (`a.empty()`), avisa que falta el parámetro.
  - Si el cliente ya está registrado, **ignora** el comando.
  - Comprueba si la contraseña es correcta: `c.passOk = (a[0] == password);`
  - Si la contraseña es incorrecta, avisa al cliente.
  - Llama a `tryRegister(c)` por si ya se cumplen todas las condiciones para registrar.

---

### **2.3. `cmdNICK(Client& c, const std::vector<std::string>& a)`**

```c++
void Server::cmdNICK(Client& c, const std::vector<std::string>& a){
    if (a.empty()) return sendTo(c, ":server NOTICE * :NICK needs parameter\r\n");
    // TODO: validar duplicados/formato
    c.nick = a[0];
    tryRegister(c);
}
```

- **Propósito:** Procesa el comando `NICK`, que asigna un apodo al cliente.
- **Lógica:**
  - Si no se pasa parámetro, avisa que falta.
  - **Nota:** Hay un `TODO` para validar si el nick está duplicado o su formato es válido (no implementado aún).
  - Asigna el nick al cliente: `c.nick = a[0];`
  - Llama a `tryRegister(c)` para intentar el registro.

---

### **2.4. `cmdUSER(Client& c, const std::vector<std::string>& a)`**

```c++
void Server::cmdUSER(Client& c, const std::vector<std::string>& a){
    if (a.size() < 4) return sendTo(c, ":server NOTICE * :USER needs 4 params\r\n");
    c.user = a[0];
    c.realname = a.back();
    tryRegister(c);
}
```

- **Propósito:** Procesa el comando `USER`, que asigna el nombre de usuario y nombre real al cliente.
- **Lógica:**
  - Si no se pasan los 4 parámetros requeridos, avisa que faltan.
  - Asigna el usuario: `c.user = a[0];`
  - Asigna el nombre real: `c.realname = a.back();`
  - Llama a `tryRegister(c)` para intentar el registro.

---

## 3. Resumen del flujo de registro

1. El cliente conecta y debe enviar los comandos `PASS`, `NICK` y `USER` (en cualquier orden).
2. Cada comando guarda la información recibida en el objeto `Client`.
3. Cada vez que se recibe uno de estos comandos, se llama a `tryRegister()`.
4. Si la contraseña es correcta, el nick y el usuario han sido definidos y el cliente no está registrado aún, se le considera registrado y se le da la bienvenida.

---
