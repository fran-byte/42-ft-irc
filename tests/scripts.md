## Configuración Inicial

Ejecuta el servidor y conecta dos clientes:

```bash
./ft_irc 6667 mypass
```

Cliente1 y Cliente2:

```bash
nc 127.0.0.1 6667
```

## Pruebas de Autenticación (Obligatorias)

Ambos clientes deben completar la secuencia de registro:

**Cliente1:**

```
PASS mypass
NICK cliente1
USER usuario1 hostname servername :Nombre Real 1
```

**Cliente2:**

```
PASS mypass
NICK cliente2
USER usuario2 hostname servername :Nombre Real 2
```

## Pruebas de Conectividad

**Comando PING:**

```
PING :test123
```

El servidor debe responder con `PONG :test123`

## Pruebas de Canales

**Comando JOIN:** [4]

- Cliente1:---> `JOIN #test` (se convierte en operador automáticamente)
- Cliente2:---> `JOIN #test` (se une como miembro regular)
- Cliente1:---> `JOIN #privado secreto` (canal con clave)

**Comando PART:** [5]

- Cliente2:---> `PART #test`
- Cliente2:---> `JOIN #test` (para volver a unirse)

## Pruebas de Mensajería

**Comando PRIVMSG:** [6]

- Cliente1:---> `PRIVMSG #test :Hola canal desde cliente1`
- Cliente2:---> `PRIVMSG #test :Respuesta desde cliente2`
- Cliente1:---> `PRIVMSG cliente2 :Mensaje privado directo`
- Cliente2:---> `PRIVMSG cliente1 :Respuesta privada`

## Pruebas de Administración de Canales

**Comando MODE:** [7]

- Cliente1:---> `MODE #test` (mostrar modos actuales)
- Cliente1:---> `MODE #test +i` (canal solo por invitación)
- Cliente1:---> `MODE #test +t` (solo operadores pueden cambiar topic)
- Cliente1:---> `MODE #test +k clave123` (establecer clave del canal)
- Cliente1:---> `MODE #test +l 5` (límite de 5 usuarios)
- Cliente1:---> `MODE #test +o cliente2` (dar privilegios de operador a cliente2)
- Cliente1:---> `MODE #test -i` (quitar modo solo invitación)
- Cliente1:---> `MODE #test -k` (quitar clave)
- Cliente1:---> `MODE #test -l` (quitar límite)
- Cliente2:---> `MODE #test -o cliente1` (quitar privilegios de operador)

**Comando TOPIC:** [8]

- Cliente1:---> `TOPIC #test` (mostrar topic actual)
- Cliente1:---> `TOPIC #test :Nuevo tema del canal de pruebas`
- Cliente2:---> `TOPIC #test :Otro tema` (debería funcionar si es operador)

**Comando INVITE:** [9]

- Cliente1:---> `MODE #test +i` (hacer canal solo por invitación)
- Cliente2:---> `PART #test` (salir del canal)
- Cliente1:---> `INVITE cliente2 #test` (invitar a cliente2)
- Cliente2:---> `JOIN #test` (ahora debería poder entrar)

**Comando KICK:** [10]

- Cliente1:---> `KICK #test cliente2 :Expulsado por pruebas`
- Cliente2 debería ser expulsado del canal automáticamente

## Pruebas de Desconexión

**Comando QUIT:** [11]

- Cliente2:---> `QUIT :Me voy del servidor`
- El servidor debe limpiar automáticamente todos los canales donde estaba cliente2

## Secuencia de Pruebas Completa

1. **Autenticación**: `PASS`, `NICK`, `USER`
2. **Conectividad**: `PING`
3. **Canales básicos**: `JOIN`, `PART`
4. **Mensajería**: `PRIVMSG` (canal y privado)
5. **Administración**: `MODE` (todos los modos: +i, +t, +k, +l, +o)
6. **Gestión de usuarios**: `INVITE`, `KICK`
7. **Configuración**: `TOPIC`
8. **Desconexión**: `QUIT`

## Notes

El servidor implementa exactamente 9 comandos como se especifica en los requisitos: [12](#1-11) `PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`, `KICK`, `INVITE`, `TOPIC`, `MODE`, más los comandos de utilidad `PING`, `QUIT` y `PART`. Todos estos comandos están manejados por el dispatcher central en `handleCommand()`.
