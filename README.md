# ft_irc



## Índice
1. [Visión General del Proyecto](#visión-general-del-proyecto)
2. [Clase Server](#clase-server)
3. [Clase Client](#clase-client)
4. [Clase Channel](#clase-channel)
5. [Parser IRC](#parser-irc)
6. [Arquitectura y Flujo de Datos](#arquitectura-y-flujo-de-datos)
7. [Comandos Implementados](#comandos-implementados)
8. [Uso y Ejecución](#uso-y-ejecución)

---

## Visión General del Proyecto

Este proyecto implementa un **servidor IRC completo en C++** que permite la comunicación entre múltiples clientes usando el protocolo IRC estándar. 

### Conceptos Técnicos Principales

El proyecto enseña conceptos fundamentales de programación de redes:
- **Sockets TCP no bloqueantes**
- **Multiplexación de clientes con poll()**
- **Buffers FIFO para envío/recepción**
- **Parsing en tiempo real de comandos IRC**
- **Gestión de canales y broadcast de mensajes** 

---

## Clase Server

### Definición y Propósito

La clase `Server` es el núcleo del sistema, responsable de gestionar todas las conexiones de clientes y procesar comandos IRC. 

### Atributos Principales

```cpp
private:
    int listen_fd;                              // Socket de escucha principal
    std::string password;                       // Contraseña del servidor
    std::vector<struct pollfd> pfds;           // Array para poll()
    std::map<int, Client> clients;             // Clientes indexados por FD
    std::map<std::string, Channel> channels;   // Canales del servidor
``` 



### Métodos de Gestión de Red

La clase implementa métodos fundamentales para la gestión de sockets:

- **Constructor**: Inicializa el puerto y contraseña 
- **`run()`**: Bucle principal del servidor 
- **`initListen()`**: Configura el socket de escucha 
- **`setNonBlocking()`**: Configura sockets no bloqueantes 

### Gestión de Clientes

El servidor maneja eventos de clientes mediante:

- **`acceptNew()`**: Acepta nuevas conexiones 
- **`handleRead()`**: Procesa datos entrantes 
- **`handleWrite()`**: Envía datos pendientes 
- **`disconnect()`**: Desconecta clientes 

### Procesamiento de Comandos

Todos los comandos IRC son manejados por métodos específicos: 

### Implementación del Bucle Principal

El método `run()` implementa el patrón de multiplexación con `poll()`:

1. **Configuración de eventos**: Cada cliente se marca para lectura, y opcionalmente para escritura si tiene datos pendientes
2. **Llamada a poll()**: Espera eventos en todos los file descriptors
3. **Procesamiento de eventos**: Maneja nuevas conexiones y eventos de clientes existentes 

---

## Clase Client

### Estructura y Propósito

La estructura `Client` representa cada usuario conectado al servidor, manteniendo su estado de conexión y buffers de comunicación. 

### Atributos del Cliente

Cada cliente mantiene:

- **`fd`**: File descriptor de la conexión
- **`recvBuf`, `sendBuf`**: Buffers de recepción y envío
- **`nick`, `user`, `realname`**: Información de identificación
- **`passOk`, `registered`**: Estados de autenticación
- **`closing`**: Bandera de desconexión
- **`channels`**: Conjunto de canales donde está el cliente 

### Constructores

La clase proporciona dos constructores:
- Constructor por defecto que inicializa valores predeterminados
- Constructor explícito que toma un file descriptor 

---

## Clase Channel

### Definición y Funcionalidad

La estructura `Channel` representa un canal IRC con todos sus modos, miembros y configuraciones. 

### Atributos del Canal

Cada canal mantiene:

- **Información básica**: `name`, `topic`, `key`
- **Modos del canal**: `inviteOnly` (+i), `topicOpOnly` (+t), `limit` (+l)
- **Listas de usuarios**: `members`, `ops`, `invited` 

### Métodos de Consulta

La clase proporciona métodos para verificar el estado de los usuarios: 

### Gestión de Modos

Métodos para configurar los diferentes modos del canal: 

### Gestión de Miembros

Operaciones para administrar la membresía del canal:  

### Validación de Acceso

El método `canJoin()` implementa la lógica completa para determinar si un usuario puede unirse al canal, verificando:

- **Límite de usuarios** (+l)
- **Clave del canal** (+k) 
- **Solo por invitación** (+i) 

---

## Parser IRC

### Estructura de Comandos

El parser define una estructura simple para representar comandos IRC:  

### Función de Parsing

La función `parseIrcLine()` implementa el parsing completo de líneas IRC:

1. **Separación de trailing**: Maneja parámetros que comienzan con ":"
2. **Extracción del verbo**: Primer token de la línea
3. **Procesamiento de argumentos**: Tokens adicionales
4. **Normalización**: Convierte el verbo a mayúsculas 

---

## Arquitectura y Flujo de Datos

### Patrón de Multiplexación

El servidor utiliza `poll()` para manejar múltiples clientes simultáneamente en un solo hilo: 

### Flujo de Recepción (POLLIN)

Cuando un cliente envía datos:

1. **Detección**: `poll()` marca `POLLIN` en el file descriptor
2. **Lectura**: `handleRead()` acumula datos en `recvBuf`
3. **Procesamiento**: Detecta líneas completas terminadas en `\r\n`
4. **Ejecución**: Llama a `onLine()` → `handleCommand()` 

### Flujo de Envío (POLLOUT)

Para enviar datos a clientes:

1. **Encolado**: `sendTo()` agrega mensajes a `sendBuf`
2. **Marcado**: Se activa `POLLOUT` para el file descriptor
3. **Envío**: `handleWrite()` envía datos cuando sea posible
4. **Gestión**: Mantiene datos no enviados para próximas iteraciones 

### Broadcast en Canales

El sistema de broadcast permite enviar mensajes a múltiples usuarios:

- **`broadcastToChannel()`**: Envía a todos los miembros excepto al emisor
- **Gestión automática**: El servidor determina a qué file descriptors enviar 

---

## Comandos Implementados

### Comandos de Autenticación

El servidor implementa la secuencia estándar de autenticación IRC:

- **`PASS`**: Verificación de contraseña 
- **`NICK`**: Establecimiento de nickname 
- **`USER`**: Información del usuario 

### Comandos de Canal

Gestión completa de canales IRC:

- **`JOIN`**: Unirse a canales 
- **`PART`**: Salir de canales 
- **`TOPIC`**: Gestión de temas 

### Comandos de Operador

Funcionalidades administrativas:

- **`MODE`**: Gestión de modos  
- **`KICK`**: Expulsar usuarios  
- **`INVITE`**: Invitar usuarios 

### Comandos de Comunicación

- **`PRIVMSG`**: Mensajes privados y de canal  
- **`PING`**: Verificación de conectividad 
- **`QUIT`**: Desconexión del servidor 

---

## Uso y Ejecución

### Compilación y Ejecución

El servidor se ejecuta desde línea de comandos:  

### Ejemplo de Uso

Para iniciar el servidor:  

### Punto de Entrada

El programa principal valida argumentos e inicializa el servidor: 

### Conexión de Clientes

Los clientes se conectan usando herramientas estándar:  

---

## Notas

Este proyecto implementa un servidor IRC completo siguiendo las especificaciones del protocolo RFC, con un diseño orientado a la eficiencia y simplicidad. La arquitectura basada en `poll()` permite manejar múltiples clientes simultáneamente sin la complejidad de múltiples hilos, mientras que el sistema de buffers asegura que no se pierdan mensajes incluso con sockets no bloqueantes.

La implementación demuestra conceptos avanzados de programación de redes, incluyendo multiplexación de E/O, gestión de estados de conexión, y parsing de protocolos en tiempo real, todo ello manteniendo un código limpio y bien estructurado.


### Citations


**File:** README.md (L4-10)


Este proyecto implementa un **servidor IRC simple en C++** para comprender los conceptos básicos de:

- Sockets TCP
- Multiplexación de clientes
- Parsing de comandos IRC

**Nota:** No incluye un cliente propio. Se espera que los clientes sean externos, como `nc`, `irssi` o cualquier otro cliente IRC.


**File:** README.md (L27-36)



### 2️⃣ Multiplexación de clientes con `poll()`

- Cada cliente se representa con un **FD** y un objeto `Client`.
- Se mantiene un vector `pfds` de `pollfd`:

  - `events` → eventos que queremos observar (lectura, escritura)
  - `revents` → eventos ocurridos

- `poll()` permite **gestionar múltiples clientes simultáneamente** sin necesidad de hilos extra.



**File:** README.md (L39-49)


### 3️⃣ Recepción de mensajes (`POLLIN`)

- Cuando un cliente envía datos, el FD marca `POLLIN`.
- El servidor llama a `handleRead()`:

  - Acumula bytes en `Client.recvBuf`
  - Detecta líneas terminadas en `\r\n`
  - Llama a `onLine()` → `handleCommand()`

- Cada línea se procesa **en tiempo real**, interpretando comandos IRC como `PRIVMSG`, `JOIN`, etc.





### 4️⃣ Envío de mensajes (`POLLOUT`)

- Los sockets son **non-blocking**, así que no siempre se puede enviar todo de golpe.
- Cada cliente tiene un **sendBuf (buffer FIFO)** con mensajes pendientes:

  - `sendTo()` agrega datos a `sendBuf` y marca `POLLOUT` para ese FD
  - Cuando `poll()` detecta `POLLOUT`, se llama a `handleWrite()`:

    - Envía bytes desde el inicio del buffer
    - Si no se puede enviar todo, se mantiene el resto para la siguiente iteración

- Esto asegura que:

  - Ningún mensaje se pierda
  - El servidor no se bloquee



**File:** README.md (L70-75)


### 5️⃣ Broadcast y canales

- Cada canal tiene un conjunto de miembros: `Channel.members`
- `broadcastToChannel()` envía mensajes a todos los miembros **excepto al emisor**
- El servidor gestiona automáticamente **a quién enviar los mensajes** según los FDs de cada cliente



**File:** README.md (L120-122)


```bash
./ft_irc <puerto> <contraseña>
```


**File:** README.md (L124-128)


**Ejemplo:**

```bash
./ft_irc 6667 mypass
```


**File:** README.md (L130-138)


- El servidor se queda en **bucle infinito** (`srv.run()`) escuchando clientes
- Los clientes externos pueden conectarse con:

```
nc 127.0.0.1 6667
```

- Todo mensaje enviado por el servidor aparecerá automáticamente en la terminal del cliente, gracias al loop interno de `nc`.



**File:** README.md (L152-159)


## Conceptos que se aprenden

- Multiplexación de clientes en **un solo hilo** usando `poll()`
- Manejo de **sockets non-blocking** sin perder mensajes
- Construcción de **buffers FIFO** por cliente (`sendBuf` y `recvBuf`)
- Implementación de un **mini parser IRC en tiempo real**
- Gestión de **canales, broadcast y mensajes privados** sin un cliente propio



**File:** include/Server.hpp (L10-14)

```text
class Server {
public:
    Server(unsigned short port, const std::string& password);
    ~Server();
    void run();
```

**File:** include/Server.hpp (L17-29)

```text
    void handleCommand(Client& c, const Cmd& cmd);
    void cmdPASS(Client& c, const std::vector<std::string>& a);
    void cmdNICK(Client& c, const std::vector<std::string>& a);
    void cmdUSER(Client& c, const std::vector<std::string>& a);
    void cmdPING(Client& c, const std::vector<std::string>& a);
    void cmdQUIT(Client& c, const std::vector<std::string>& a);
    void cmdJOIN(Client& c, const std::vector<std::string>& a);
    void cmdPART(Client& c, const std::vector<std::string>& a);
    void cmdPRIVMSG(Client& c, const std::vector<std::string>& a);
    void cmdMODE (Client& c, const std::vector<std::string>& a);
    void cmdTOPIC(Client& c, const std::vector<std::string>& a);
    void cmdINVITE(Client& c, const std::vector<std::string>& a);
    void cmdKICK (Client& c, const std::vector<std::string>& a);
```

**File:** include/Server.hpp (L35-39)

```text
    int listen_fd;
    std::string password;
    std::vector<struct pollfd> pfds;
    std::map<int, Client> clients;
    std::map<std::string, Channel> channels;
```

**File:** include/Server.hpp (L41-41)

```text
    void initListen(unsigned short port);
```

**File:** include/Server.hpp (L42-42)

```text
    void setNonBlocking(int fd);
```

**File:** include/Server.hpp (L43-43)

```text
    void acceptNew();
```

**File:** include/Server.hpp (L44-44)

```text
    void handleRead(size_t idx);
```

**File:** include/Server.hpp (L45-45)

```text
    void handleWrite(size_t idx);
```

**File:** include/Server.hpp (L46-46)

```text
    void disconnect(size_t idx);
```

**File:** src/core/Server.cpp (L37-50)

```cpp
void Server::run(){
    for(;;){
        for (size_t i=1;i<pfds.size();++i){
            int fd = pfds[i].fd;
            pfds[i].events = clients[fd].sendBuf.empty()? POLLIN : (POLLIN|POLLOUT);
        }
        int n = ::poll(&pfds[0], pfds.size(), -1);
        if (n < 0){ if (errno==EINTR) continue; throw std::runtime_error("poll failed"); }

        if (pfds[0].revents & POLLIN) acceptNew();

        for (size_t i=1;i<pfds.size();++i){
            short ev = pfds[i].revents;
            if (ev & (POLLERR|POLLHUP|POLLNVAL)) { disconnect(i--); continue; }
```

**File:** include/Client.hpp (L5-15)

```text
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

**File:** include/Channel.hpp (L5-15)

```text
struct Channel {
    std::string name, topic, key;
    bool inviteOnly;     // +i
    bool topicOpOnly;    // +t
    int  limit;          // +l; -1 = sin límite
    std::set<int> members, ops, invited;

    Channel() : inviteOnly(false), topicOpOnly(false), limit(-1) {}
    explicit Channel(const std::string& n)
        : name(n), inviteOnly(false), topicOpOnly(false), limit(-1) {}

```

**File:** include/Channel.hpp (L17-19)

```text
    bool isMember(int fd)   const { return members.count(fd)   != 0; }
    bool isOp(int fd)       const { return ops.count(fd)       != 0; }
    bool isInvited(int fd)  const { return invited.count(fd)   != 0; }
```

**File:** include/Channel.hpp (L22-27)

```text
    void setInviteOnly(bool on)      { inviteOnly = on; }
    void setTopicRestricted(bool on) { topicOpOnly = on; }
    void setKey(const std::string& k){ key = k; }
    void clearKey()                  { key.clear(); }
    void setLimit(int n)             { limit = n; }
    void clearLimit()                { limit = -1; }
```

**File:** include/Channel.hpp (L30-33)

```text
    void addMember(int fd)   { members.insert(fd); invited.erase(fd); }
    void removeMember(int fd){ members.erase(fd); ops.erase(fd); invited.erase(fd); }
    void addOp(int fd)       { ops.insert(fd); }
    void invite(int fd)      { invited.insert(fd); }
```

**File:** include/Channel.hpp (L36-44)

```text
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

**File:** include/Parser.hpp (L6-6)

```text
struct Cmd { std::string verb; std::vector<std::string> args; };
```

**File:** include/Parser.hpp (L8-18)

```text
inline Cmd parseIrcLine(const std::string& line) {
    Cmd out; std::string s = line, trailing;
    std::string::size_type p = s.find(" :");
    if (p != std::string::npos) { trailing = s.substr(p + 2); s.erase(p); }
    std::istringstream iss(s);
    iss >> out.verb;
    for (std::string tok; iss >> tok; ) out.args.push_back(tok);
    if (!trailing.empty()) out.args.push_back(trailing);
    for (size_t i=0;i<out.verb.size();++i) out.verb[i]=std::toupper(out.verb[i]);
    return out;
}
```

**File:** src/main.cpp (L5-19)

```cpp
int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>\n";
        return 1;
    }
    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));
    std::string password = argv[2];
    try {
        Server srv(port, password);
        srv.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
```

