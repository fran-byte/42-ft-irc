- Este archivo implementa la clase `Server`, que es el núcleo del servidor IRC (Internet Relay Chat). Aborda la administración de clientes, conexiones, canales y la comunicación entre ellos usando sockets en C++.

---

## 1. **Includes y dependencias**

Incluye cabeceras estándar de C++ y POSIX para trabajar con sockets, errores, punteros, polling, etc.  
Incluye `"Server.hpp"` para las definiciones de la clase y estructuras auxiliares.

---

## 2. **Constructor y Destructor**

```c++
Server::Server(unsigned short port, const std::string& pwd)
: listen_fd(-1), password(pwd) {
    initListen(port);
    struct pollfd p; p.fd = listen_fd; p.events = POLLIN; p.revents = 0;
    pfds.push_back(p);
}
Server::~Server(){ if (listen_fd>=0) ::close(listen_fd); }
```

- **Constructor**: Crea el socket de escucha, lo configura y lo añade a la lista de "pollfds" (`pfds`) para ser monitoreado.
- **Destructor**: Cierra el socket de escucha si está abierto.

---

## 3. **Sockets en modo no bloqueante**

```c++
void Server::setNonBlocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) flags = 0;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
```

Esto hace que el descriptor de archivo (`fd`) no bloquee al leer o escribir, muy útil para servidores concurrents.

---

## 4. **Inicialización del socket de escucha**

```c++
void Server::initListen(unsigned short port){
    listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    ...
    setNonBlocking(listen_fd);
}
```

- Crea el socket.
- Usa `setsockopt` para permitir reutilización de la dirección.
- Asocia el socket a todas las interfaces (`INADDR_ANY`) y al puerto.
- Comienza a escuchar (`listen`).
- Lo pone en modo no bloqueante.

---

## 5. **Bucle principal del servidor**

```c++
void Server::run(){
    for(;;){
        ...
        int n = ::poll(&pfds[0], pfds.size(), -1);
        ...
        if (pfds[0].revents & POLLIN) acceptNew();
        ...
        for (size_t i=1;i<pfds.size();++i){
            ...
            if (ev & POLLIN)  handleRead(i);
            if (i<pfds.size() && (pfds[i].revents & POLLOUT)) handleWrite(i);
        }
        ...
    }
}
```

- Usa `poll` para esperar eventos en los sockets.
- Si hay conexión entrante, llama a `acceptNew()`.
- Para cada cliente, gestiona lectura/escritura según los eventos.
- Cierra conexiones cuya salida ya ha sido enviada (`sendBuf` vacío y marcado como cerrando).

---

## 6. **Aceptar nuevos clientes**

```c++
void Server::acceptNew(){
    for(;;){
        int cfd = ::accept(listen_fd, NULL, NULL);
        ...
        setNonBlocking(cfd);
        clients.insert(std::make_pair(cfd, Client(cfd)));
        ...
        pfds.push_back(p);
        std::fprintf(stderr, "Nuevo cliente (fd=%d)\n", cfd);
    }
}
```

- Acepta todas las conexiones pendientes.
- Cada cliente se añade al mapa `clients` y a la lista `pfds` para ser monitoreado.
- El socket se pone en modo no bloqueante.

---

## 7. **Lectura de clientes**

```c++
void Server::handleRead(size_t idx){
    int fd = pfds[idx].fd;
    ...
    for(;;){
        ssize_t r = ::recv(fd, buf, sizeof(buf), 0);
        if (r > 0){
            c.recvBuf.append(buf, r);
            ...
            while ((pos = c.recvBuf.find("\r\n")) != std::string::npos) {
                std::string line = c.recvBuf.substr(0, pos);
                c.recvBuf.erase(0, pos + 2);
                onLine(c, line);
            }
        } ...
    }
}
```

- Lee todos los datos disponibles.
- Acumula en un buffer (`recvBuf`) por si los mensajes llegan fragmentados.
- Procesa línea por línea (comandos IRC terminan en `\r\n`).
- Si el cliente cierra la conexión, llama a `disconnect`.

---

## 8. **Envío de datos a clientes**

```c++
void Server::handleWrite(size_t idx){
    int fd = pfds[idx].fd;
    ...
    while (!c.sendBuf.empty()){
        ssize_t n = ::send(fd, c.sendBuf.data(), c.sendBuf.size(), 0);
        if (n > 0) c.sendBuf.erase(0, n);
        ...
    }
    pfds[idx].events &= ~POLLOUT;
}
```

- Mientras haya datos para enviar en `sendBuf`, los envía.
- Si el buffer queda vacío, desactiva el evento POLLOUT para ese cliente.

---

## 9. **Desconexión de clientes**

```c++
void Server::disconnect(size_t idx){
    int fd = pfds[idx].fd;
    ...
    if (it != clients.end() && !it->second.channels.empty()) {
        quitCleanup(it->second, "Connection closed");
    }
    ::close(fd);
    clients.erase(fd);
    pfds[idx] = pfds.back(); pfds.pop_back();
}
```

- Si el cliente estaba en algún canal, hace limpieza.
- Cierra el socket, elimina al cliente y lo quita de `pfds`.

---

## 10. **Envío y marcado para escritura**

```c++
void Server::sendTo(Client& c, const std::string& msg){ c.sendBuf += msg; markWrite(c.fd); }
void Server::markWrite(int fd){ for(size_t i=1;i<pfds.size();++i) if (pfds[i].fd==fd){ pfds[i].events |= POLLOUT; break; } }
```

- Añade un mensaje al buffer de salida del cliente y activa POLLOUT para que el servidor sepa que tiene que escribirle.

---

## 11. **Procesamiento de líneas y comandos**

```c++
void Server::onLine(Client& c, const std::string& line){ Cmd cmd = parseIrcLine(line); handleCommand(c, cmd); }
```

- Recibe una línea de texto (comando IRC), la parsea y la pasa a la función que la ejecuta.

---

## 12. **Búsqueda de clientes y broadcasting**

```c++
int Server::findFdByNick(const std::string& nick) const { ... }
void Server::broadcastToChannel(const Channel& ch, int fromFd, const std::string& msg){ ... }
```

- Búsqueda de un cliente por su nick.
- Envío de un mensaje a todos los miembros de un canal, excepto el emisor.

---

## 13. **Limpieza al salir (QUIT)**

```c++
void Server::quitCleanup(Client& c, const std::string& reason){
    ...
    for (size_t i = 0; i < chans.size(); ++i) {
        ...
        broadcastToChannel(ch, c.fd, quitMsg);
        ch.removeMember(c.fd);
        c.channels.erase(chans[i]);
        if (ch.members.empty()) channels.erase(it);
        else if (ch.ops.empty()) {
            int promote = *ch.members.begin();
            ch.addOp(promote);
        }
    }
}
```

- Envía un mensaje QUIT a los canales donde estaba el cliente.
- Elimina al cliente de los canales y borra los canales vacíos.
- Si no quedan operadores en el canal, promueve a uno de los miembros.

---

## **Resumen**

- Implementa un servidor IRC con sockets no bloqueantes y multiplexación con `poll`.
- Permite gestionar múltiples clientes y canales de comunicación simultáneamente.
- Gestiona la entrada/salida de datos, conexiones y desconexiones, y la comunicación entre clientes de manera eficiente y segura.
