- `Server.hpp` definición de la clase `Server`, que representa el corazón del servidor IRC (Internet Relay Chat) encargado de manejar conexiones, comandos y comunicación entre clientes y canales.

---

## Detalle de la clase `Server`

### Librerías incluidas

- **<map>** y **<vector>**: Estructuras de datos para almacenar clientes y canales.
- **<string>**: Para manipulación de cadenas.
- **<poll.h>**: Para el manejo de múltiples conexiones de red.
- **"Client.hpp"**, **"Channel.hpp"**, **"Parser.hpp"**: Cabeceras propias del proyecto para gestionar clientes, canales y el parser de comandos.

---

### Métodos públicos

#### **Constructor y Destructor**

- `Server(unsigned short port, const std::string& password);`
  - Inicializa el servidor en el puerto y con la contraseña especificada.
- `~Server();`
  - Libera los recursos del servidor.

#### **run**

- `void run();`
  - Lanza el bucle principal del servidor, gestionando conexiones y comandos.

#### **Manejo de comandos IRC**

- Métodos del tipo `cmdNICK`, `cmdUSER`, etc.:

  - Cada uno gestiona un comando específico del protocolo IRC. Por ejemplo:
    - `cmdNICK`: Cambiar el apodo de un usuario.
    - `cmdPASS`: Verificar la contraseña de conexión.
    - `cmdJOIN`: Unirse a un canal.
    - `cmdPRIVMSG`: Enviar mensajes privados.
    - etc.

- `void handleCommand(Client& c, const Cmd& cmd);`
  - Recibe un comando de un cliente y lo despacha al manejador adecuado.

#### **Comunicación**

- `void sendTo(Client& c, const std::string& msg);`
  - Envía un mensaje a un cliente específico.
- `void markWrite(int fd);`
  - Marca un descriptor para que sea escrito (útil en I/O no bloqueante).

---

### Atributos privados

- `int listen_fd;`
  - Descriptor del socket de escucha (donde llegan nuevas conexiones).
- `std::string password;`
  - Contraseña que los clientes deben usar para conectarse.
- `std::vector<struct pollfd> pfds;`
  - Lista de descriptores de archivos usados con `poll()` para gestionar múltiples conexiones simultáneamente.
- `std::map<int, Client> clients;`
  - Mapa de todos los clientes conectados, indexados por su descriptor de archivo.
- `std::map<std::string, Channel> channels;`
  - Mapa de canales existentes, indexados por nombre.

---

### Métodos privados

- **Inicialización y control de sockets:**

  - `void initListen(unsigned short port);`  
    Prepara el socket para escuchar conexiones.
  - `void setNonBlocking(int fd);`  
    Configura un socket para que las operaciones no bloqueen.

- **Manejo de conexiones y eventos:**

  - `void acceptNew();`  
    Acepta una nueva conexión de cliente.
  - `void handleRead(size_t idx);`  
    Lee datos de un cliente.
  - `void handleWrite(size_t idx);`  
    Envía datos pendientes a un cliente.
  - `void disconnect(size_t idx);`  
    Desconecta y limpia los recursos de un cliente.

- **Manejo de lógica de protocolo IRC:**
  - `void onLine(Client& c, const std::string& line);`  
    Procesa una línea de comando recibida.
  - `void tryRegister(Client& c);`  
    Intenta completar el registro del cliente cuando este ha enviado PASS, NICK y USER.
  - `int findFdByNick(const std::string& nick) const;`  
    Busca el descriptor de archivo de un cliente por su apodo.
  - `void broadcastToChannel(const Channel& ch, int fromFd, const std::string& msg);`  
    Envía un mensaje a todos los miembros de un canal, menos al remitente.
  - `void quitCleanup(Client& c, const std::string& reason);`  
    Limpia y notifica la salida de un cliente.

---

## Resumen

La clase `Server` es el controlador principal de un servidor IRC. Gestiona conexiones concurrentes usando `poll`, maneja clientes y canales, interpreta comandos del protocolo IRC, y se encarga de la comunicación entre usuarios y canales.

Cada método está pensado para manejar una parte del flujo de trabajo IRC: desde la llegada de la conexión, el registro del usuario, la interpretación de comandos, el envío de mensajes, hasta la desconexión y limpieza de recursos.
