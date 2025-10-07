- `InviteKick.cpp` contiene la implementación de los comandos IRC `INVITE` y `KICK` en el contexto de un servidor IRC.

---

## Resumen General

Este archivo define dos métodos de la clase `Server`:

- `cmdINVITE`: gestiona el comando IRC `/INVITE` para invitar a un usuario a un canal.
- `cmdKICK`: gestiona el comando IRC `/KICK` para expulsar a un usuario de un canal.

Ambos comandos comprueban permisos y el estado del canal/usuarios antes de realizar la acción y notifican tanto a los involucrados como a los demás usuarios del canal.

---

## Explicación Detallada

### 1. Método `Server::cmdINVITE`

#### Propósito

Permite a un usuario invitar a otro usuario a un canal, siguiendo las reglas de permisos habituales de IRC.

#### Parámetros

- `Client& c`: referencia al cliente que ejecuta el comando.
- `const std::vector<std::string>& a`: argumentos del comando (`a[0]` es el nick invitado, `a[1]` es el canal).

#### Lógica paso a paso

1. **Validación de argumentos**

   - Si no hay suficientes argumentos (`<nick> <#chan>`), muestra un mensaje de ayuda.

2. **Obtención de argumentos**

   - `nick`: nick del usuario a invitar.
   - `chan`: string del canal.

3. **Verifica que el canal existe**

   - Busca el canal en el mapa `channels`.
   - Si el canal no existe, avisa al usuario.

4. **Verifica que el invitador es miembro del canal**

   - Solo se puede invitar a alguien a un canal si eres miembro.

5. **Verifica permisos de operador en canales +i (invite-only)**

   - Si el canal es de invitación (`inviteOnly == true`), solo los operadores pueden invitar.

6. **Busca el file descriptor del usuario a invitar**

   - Usa una función `findFdByNick` para encontrar el `fd` del nick objetivo.
   - Si no existe, avisa al usuario.

7. **Invita al usuario**

   - Llama a `ch.invite(tfd)` para marcarlo como invitado.

8. **Notifica al invitado**

   - Si el usuario existe en el mapa `clients`, le envía un mensaje de invitación.

9. **Notifica al invitador**
   - Envía un mensaje de confirmación.

#### Resumen gráfico de flujo

```
Cliente ejecuta INVITE --> ¿Argumentos válidos? --> ¿Canal existe? --> ¿Eres miembro? --> (¿Invite-only? --> ¿Eres op?) --> ¿Nick existe? --> Invitar --> Notificar invitado + invitador
```

---

### 2. Método `Server::cmdKICK`

#### Propósito

Permite a un operador de canal expulsar a un usuario de un canal.

#### Parámetros

- `Client& c`: referencia al cliente que ejecuta el comando.
- `const std::vector<std::string>& a`: argumentos del comando (`a[0]` es el canal, `a[1]` es el nick a expulsar, `a[2]` (opcional) es la razón).

#### Lógica paso a paso

1. **Validación de argumentos**

   - Si no hay suficientes argumentos (`<#chan> <nick>`), muestra un mensaje de ayuda.

2. **Obtención de argumentos**

   - `chan`: canal objetivo.
   - `nick`: nick del usuario a expulsar.
   - `reason`: razón de la expulsión (opcional, por defecto "Kicked").

3. **Verifica que el canal existe**

   - Busca el canal en el mapa `channels`.
   - Si no existe, avisa al usuario.

4. **Verifica que el usuario es operador**

   - Solo los operadores pueden expulsar miembros.

5. **Busca el fd del usuario a expulsar y verifica que esté en el canal**

   - Si el usuario no está en el canal, avisa.

6. **Aviso a todos los miembros del canal**

   - Construye el mensaje de KICK y lo envía a todos los miembros del canal.

7. **Elimina al usuario del canal**

   - Llama a `ch.removeMember(tfd)` y elimina el canal de la lista de canales del usuario.

8. **Elimina el canal si ya no tiene miembros**
   - Si tras la expulsión no quedan miembros, elimina el canal del mapa `channels`.

#### Resumen gráfico de flujo

```
Cliente ejecuta KICK --> ¿Argumentos válidos? --> ¿Canal existe? --> ¿Eres operador? --> ¿Nick existe y está en canal? --> Avisar a canal --> Quitar usuario --> (¿Canal vacío? --> Eliminar canal)
```

---

## Notas adicionales

- `sendTo(c, mensaje)`: método para enviar un mensaje a un cliente.
- `broadcastToChannel(ch, -1, msg)`: envía el mensaje a todos los miembros del canal (excepto opcionalmente uno, aquí -1 significa a todos).
- Los permisos y la gestión de miembros y operadores están delegados en la clase `Channel`.

---
