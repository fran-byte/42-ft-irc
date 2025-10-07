- `Server::cmdPRIVMSG` en el archivo `Message.cpp` del proyecto IRC:

---

## ¿Qué hace este método?

El método `cmdPRIVMSG` implementa el comando **PRIVMSG** del protocolo IRC, que sirve para enviar mensajes privados a otros usuarios o a canales.

---

## Paso a paso del código

1. **Comprobación de argumentos:**

   ```c++
   if (a.size() < 2)
       return sendTo(c, ":server NOTICE " + c.nick + " :PRIVMSG needs <target> :<text>\r\n");
   ```

   - Si el número de argumentos es menor a 2 (es decir, falta el destinatario o el texto), se le notifica al cliente que faltan argumentos.

2. **Obtención del target y el texto:**

   ```c++
   const std::string& target = a[0];
   const std::string& text   = a[1]; // nuestro parser mete el trailing aquí
   ```

   - `target` será el destinatario del mensaje (puede ser un canal o un usuario).
   - `text` es el mensaje a enviar.

3. **Detección si el destinatario es un canal:**

   ```c++
   if (!target.empty() && target[0] == '#') {
   ```

   - En IRC, los canales siempre empiezan con `#`.

   a. **Buscar el canal:**

   ```c++
   std::map<std::string, Channel>::iterator it = channels.find(target);
   if (it == channels.end())
       return sendTo(c, ":server NOTICE " + c.nick + " :No such channel\r\n");
   ```

   - Si el canal no existe, se notifica al usuario.

   b. **Comprobar si el usuario es miembro del canal:**

   ```c++
   Channel &ch = it->second;
   if (!ch.isMember(c.fd))
       return sendTo(c, ":server NOTICE " + c.nick + " :Cannot send to channel\r\n");
   ```

   - Si el usuario no es miembro del canal, no puede enviar mensajes a él.

   c. **Construir y enviar el mensaje al canal:**

   ```c++
   std::string msg = ":" + c.nick + " PRIVMSG " + target + " :" + text + "\r\n";
   broadcastToChannel(ch, c.fd, msg);
   return;
   ```

   - Se forma el mensaje en formato IRC y se envía a todos los miembros del canal (excepto al emisor).

4. **Si el destinatario es un usuario:**

   ```c++
   else {
       int dstFd = findFdByNick(target);
       if (dstFd < 0)
           return sendTo(c, ":server NOTICE " + c.nick + " :No such nick\r\n");
   ```

   - Se busca el descriptor de archivo (`fd`) del usuario destino por su nick.
   - Si no se encuentra, se notifica al remitente.

   a. **Comprobar que el usuario sigue conectado:**

   ```c++
   std::map<int, Client>::iterator itc = clients.find(dstFd);
   if (itc == clients.end()) return; // se desconectó
   ```

   - Si el usuario se desconectó, simplemente termina la función.

   b. **Enviar el mensaje privado:**

   ```c++
   std::string msg = ":" + c.nick + " PRIVMSG " + target + " :" + text + "\r\n";
   sendTo(itc->second, msg);
   return;
   ```

   - Se construye el mensaje y se envía directamente al usuario destino.

---

## Resumen

- El método distingue entre mensajes a canales y a usuarios.
- Valida que el destinatario exista y que se tengan permisos para enviar el mensaje.
- Envía el mensaje en formato IRC apropiado, usando funciones auxiliares como `sendTo` y `broadcastToChannel`.
- Maneja errores comunes devolviendo mensajes de tipo NOTICE al usuario si hay problemas.
