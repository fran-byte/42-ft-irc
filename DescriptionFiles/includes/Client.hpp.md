- `include/Client.hpp`:

---

### 1. **Directivas de preprocesador**

```c++
#pragma once
#include <string>
#include <set>
```

- `#pragma once`: Evita que este archivo de cabecera se incluya más de una vez en el mismo archivo de compilación (protección contra inclusiones múltiples).
- `#include <string>`: Permite usar la clase `std::string` para manejar cadenas de texto.
- `#include <set>`: Permite usar la clase `std::set`, que es un contenedor de la STL que almacena elementos únicos de forma ordenada.

---

### 2. **Definición de la estructura `Client`**

```c++
struct Client {
    int fd;
    std::string recvBuf, sendBuf;
    std::string nick, user, realname;
    bool passOk, registered;
    bool closing;
    std::set<std::string> channels;

    Client(): fd(-1), passOk(false), registered(false), closing(false) {}
    explicit Client(int f): fd(f), passOk(false), registered(false), closing(false) {}
};
```

#### **Atributos:**

- `int fd;`
  - **Descripción:** File descriptor (descriptor de archivo). Normalmente representa la conexión de red (socket) asociada al cliente.
- `std::string recvBuf, sendBuf;`
  - **recvBuf:** Buffer para almacenar datos recibidos desde el cliente (aún no procesados).
  - **sendBuf:** Buffer para almacenar datos que se van a enviar al cliente (pendientes de envío).
- `std::string nick, user, realname;`
  - **nick:** Apodo (nickname) del cliente en el servidor IRC.
  - **user:** Nombre de usuario (identificador).
  - **realname:** Nombre real del usuario.
- `bool passOk, registered;`
  - **passOk:** Indica si el cliente ha pasado correctamente la autenticación de contraseña.
  - **registered:** Indica si el cliente se ha registrado correctamente en el servidor IRC.
- `bool closing;`
  - **closing:** Indica si el cliente está en proceso de desconexión/cierre.
- `std::set<std::string> channels;`
  - **channels:** Conjunto de canales a los que el cliente está suscrito. El uso de `set` garantiza que no haya canales repetidos y que estén ordenados alfabéticamente.

#### **Constructores:**

- **Constructor por defecto:**

  ```c++
  Client(): fd(-1), passOk(false), registered(false), closing(false) {}
  ```

  - Inicializa el descriptor de archivo a `-1` (valor inválido).
  - Inicializa los booleanos a `false`.
  - Los strings y el set se inicializan vacíos por defecto.

- **Constructor con parámetro:**
  ```c++
  explicit Client(int f): fd(f), passOk(false), registered(false), closing(false) {}
  ```
  - Inicializa el descriptor de archivo a `f` (valor recibido como argumento).
  - El resto de los atributos igual que el constructor por defecto.

---

### **Resumen del contexto**

Esta estructura representa a cada cliente conectado a un servidor IRC (Internet Relay Chat).

- Permite guardar el estado de conexión, los datos de identificación, los canales a los que pertenece y buffers para gestión de mensajes.
- Es una estructura fundamental para el funcionamiento interno de un servidor IRC, ya que encapsula toda la información relevante de un usuario conectado.
