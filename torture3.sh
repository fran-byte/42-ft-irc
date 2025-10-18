#!/bin/bash
# torture2.sh - Pruebas MEJORADAS con verificación de respuestas 🚀

SERVER_HOST="127.0.0.1"
SERVER_PORT="6667"
PASSWORD="mypass"

echo "🎯 PRUEBAS MEJORADAS - CON VERIFICACIÓN DE RESPUESTAS 🎯"
echo "========================================================"
echo ""

# Función para verificar servidor
check_server() {
    ps aux | grep -v grep | grep -q -E "(valgrind.*ircserv|./ircserv)"
}

# Función MEJORADA con verificación de respuestas
run_verified_client() {
    local client_id="$1"
    local commands="$2"
    local expected_responses="$3"
    local duration="${4:-10}"
    
    local responses_received=0
    local expected_count=$(echo "$expected_responses" | grep -v '^#' | grep -v '^$' | wc -l)
    
    (
        echo "🔧 ========================================"
        echo "👤 CLIENTE: $client_id - INICIADO 🟢"
        echo "🔧 ========================================"
        
        echo "📋 COMANDOS:"
        echo "$commands" | grep -v '^#' | sed '/^$/d'
        echo ""
        echo "✅ RESPUESTAS ESPERADAS:"
        echo "$expected_responses" | grep -v '^#' | sed '/^$/d'
        echo ""
        echo "📡 --- TRAFICO EN VIVO ---"
        
        # Crear archivos temporales
        local temp_commands=$(mktemp)
        local temp_responses=$(mktemp)
        
        # Procesar comandos
        {
            while IFS= read -r line; do
                if [[ -n "$line" && ! "$line" =~ ^[[:space:]]*# ]]; then
                    if [[ "$line" == sleep* ]]; then
                        echo "$line"
                    elif [[ "$line" =~ ^for ]]; then
                        echo "$line"
                    elif [[ "$line" == done* ]]; then
                        echo "$line"
                    else
                        echo "printf '%s\\r\\n' '$line'"
                    fi
                fi
            done <<< "$commands"
        } > "$temp_commands"
        chmod +x "$temp_commands"
        
        # Ejecutar y capturar respuestas
        {
            bash "$temp_commands"
        } | {
            nc -C -w 10 $SERVER_HOST $SERVER_PORT 2>&1 | tee "$temp_responses" | while IFS= read -r response; do
                if [[ ! "$response" =~ "Connection to" ]] && [[ ! "$response" =~ "succeeded" ]] && [[ -n "$response" ]]; then
                    echo "📥 <<< $client_id RECIBE: $response"
                    
                    # Verificar si es una respuesta esperada
                    while IFS= read -r expected; do
                        if [[ -n "$expected" && ! "$expected" =~ ^[[:space:]]*# ]] && [[ "$response" =~ $expected ]]; then
                            echo "   ✅ RESPUESTA ESPERADA DETECTADA: '$expected'"
                            ((responses_received++))
                        fi
                    done <<< "$expected_responses"
                fi
            done
        } &
        
        local client_pid=$!
        sleep $duration
        kill $client_pid 2>/dev/null
        wait $client_pid 2>/dev/null
        
        echo "📡 --- FIN TRAFICO ---"
        
        # Análisis de respuestas
        echo ""
        echo "📊 ANALISIS DE RESPUESTAS:"
        echo "   📈 Respuestas esperadas: $expected_count"
        echo "   ✅ Respuestas recibidas: $responses_received"
        
        if [ $responses_received -eq $expected_count ] && [ $expected_count -gt 0 ]; then
            echo "   🎉 TODAS las respuestas esperadas recibidas!"
        elif [ $expected_count -eq 0 ]; then
            echo "   ℹ️  No se definieron respuestas esperadas para verificar"
        else
            echo "   ⚠️  Faltan respuestas: $((expected_count - responses_received))"
            echo "   📋 Respuestas no recibidas:"
            while IFS= read -r expected; do
                if [[ -n "$expected" && ! "$expected" =~ ^[[:space:]]*# ]]; then
                    if ! grep -q "$expected" "$temp_responses" 2>/dev/null; then
                        echo "      ❌ '$expected'"
                    fi
                fi
            done <<< "$expected_responses"
        fi
        rm -f "$temp_commands" "$temp_responses"
        echo "👤 CLIENTE: $client_id - FINALIZADO 🔴"
        echo "🔧 ========================================"
        echo ""
        
        # Retornar éxito si se recibieron todas las respuestas esperadas
        if [ $responses_received -eq $expected_count ]; then
            return 0
        else
            return 1
        fi
    ) 
}

# Función para conexiones silenciosas (sin output)
run_silent_client() {
    local client_id="$1"
    local commands="$2"
    local duration="${3:-10}"
    
    (
        local temp_file=$(mktemp)
        {
            while IFS= read -r line; do
                if [[ -n "$line" && ! "$line" =~ ^[[:space:]]*# ]]; then
                    if [[ "$line" == sleep* ]]; then
                        echo "$line"
                    else
                        echo "printf '%s\\r\\n' '$line'"
                    fi
                fi
            done <<< "$commands"
        } > "$temp_file"
        chmod +x "$temp_file"
        
        bash "$temp_file" | nc -C -w 10 $SERVER_HOST $SERVER_PORT > /dev/null 2>&1 &
        
        local client_pid=$!
        sleep $duration
        kill $client_pid 2>/dev/null
        wait $client_pid 2>/dev/null
        rm -f "$temp_file"
    ) 
}

# Función para monitorear rechazos
monitor_rejection() {
    local client_id="$1"
    local client_num="$2"
    
    temp_file=$(mktemp)
    {
        echo "printf 'PASS $PASSWORD\\r\\n'"
        echo "printf 'NICK overload$client_num\\r\\n'"
        echo "printf 'USER ov$client_num 0 * :Overload User $client_num\\r\\n'"
        echo "sleep 5"
    } > "$temp_file"
    chmod +x "$temp_file"
    
    bash "$temp_file" | nc -C -w 8 $SERVER_HOST $SERVER_PORT 2>&1 | grep -q "ERROR\|full\|reject"
    if [ $? -eq 0 ]; then
        echo "   🔥 overload$client_num: RECHAZADO (esperado ✅)"
        echo "1" >> /tmp/rejected_count_$$
    else
        echo "   ⚠️  overload$client_num: CONECTADO (inesperado)"
    fi
    rm -f "$temp_file"
}

cleanup() {
    echo "🧹 Limpiando procesos..."
    pkill -f "nc $SERVER_HOST $SERVER_PORT" 2>/dev/null
    rm -f /tmp/rejected_count_$$ 2>/dev/null
    sleep 2
}

trap cleanup EXIT

# Verificación inicial
echo "🔍 Verificando servidor..."
if check_server; then
    echo "✅ Servidor detectado 👍"
else
    echo "❌ ERROR: Servidor no detectado 👎"
    exit 1
fi

# Función de espera mejorada
wait_and_check() {
    local wait_time=$1
    local test_name=$2
    
    echo ""
    echo "⚡ EJECUTANDO: $test_name ($wait_time segundos) ⏳"
    for i in $(seq 1 $wait_time); do
        sleep 1
        if ! check_server; then
            echo "💥 ERROR: Servidor crasheo 🚨"
            return 1
        fi
    done
    echo "✅ COMPLETADO: $test_name 🎉"
    return 0
}
# Contadores
FAILED_TESTS=0
TOTAL_TESTS=10
RESPONSE_FAILURES=0

echo ""
echo "🚀 INICIANDO PRUEBAS MEJORADAS..."
echo ""

# PRUEBA 1: Autenticación básica CON VERIFICACIÓN
echo "1️⃣ /$TOTAL_TESTS: AUTENTICACION BASICA 🔐"
for i in {1..2}; do
    commands=$(cat <<CMD
PASS $PASSWORD
NICK user$i
USER u$i 0 * :User $i
sleep 1
PING :test$i
sleep 2
QUIT :Bye
CMD
)

    expected_responses=$(cat <<RESP
.*001
.*PONG :test$i
RESP
)

    if ! run_verified_client "user$i" "$commands" "$expected_responses" 6; then
        ((RESPONSE_FAILURES++))
    fi &
    sleep 1
done

wait_and_check 8 "autenticacion con verificacion" || ((FAILED_TESTS++))

# PRUEBA 2: Canales simples CON VERIFICACIÓN
echo ""
echo "2️⃣ /$TOTAL_TESTS: CANALES SIMPLES 💬"
commands=$(cat <<CMD
PASS $PASSWORD
NICK tester
USER te 0 * :Tester
sleep 1
JOIN #test
PRIVMSG #test :Hola canal
sleep 2
TOPIC #test :Mi topico
sleep 1
PRIVMSG #test :Segundo mensaje
sleep 2
PART #test :Adios
sleep 1
CMD
)

expected_responses=$(cat <<RESP
.*JOIN #test
.*PRIVMSG #test :Hola canal
.*TOPIC #test :Mi topico
.*PART #test :Adios
RESP
)

if ! run_verified_client "tester" "$commands" "$expected_responses" 8; then
    ((RESPONSE_FAILURES++))
fi

wait_and_check 10 "canales con verificacion" || ((FAILED_TESTS++))

# PRUEBA 3: Comandos básicos CON VERIFICACIÓN
echo ""
echo "3️⃣ /$TOTAL_TESTS: COMANDOS BASICOS 🛠️"
commands=$(cat <<CMD
PASS $PASSWORD
NICK cmduser
USER cu 0 * :Command User
sleep 1
JOIN #room1
JOIN #room2
sleep 1
PRIVMSG #room1 :Hola room1
PRIVMSG #room2 :Hola room2
sleep 2
PART #room1 :Saliendo de room1
PART #room2 :Saliendo de room2
sleep 1
CMD
)

expected_responses=$(cat <<RESP
.*JOIN #room1
.*JOIN #room2
.*PRIVMSG #room1 :Hola room1
.*PRIVMSG #room2 :Hola room2
RESP
)

if ! run_verified_client "cmduser" "$commands" "$expected_responses" 8; then
    ((RESPONSE_FAILURES++))
fi

wait_and_check 10 "comandos basicos con verificacion" || ((FAILED_TESTS++))
# PRUEBA 4: Interacción entre usuarios CON VERIFICACIÓN
echo ""
echo "4️⃣ /$TOTAL_TESTS: INTERACCION ENTRE USUARIOS 👥"

# Cliente 1
commands1=$(cat <<CMD
PASS $PASSWORD
NICK userA
USER ua 0 * :User A
sleep 1
JOIN #chat
PRIVMSG #chat :Hola soy UserA
sleep 3
PRIVMSG #chat :Alguien me escucha?
sleep 3
CMD
)

expected1=$(cat <<RESP
.*JOIN #chat
.*PRIVMSG #chat :Hola UserA! Soy UserB
.*PRIVMSG #chat :Te escucho claro
RESP
)

# Cliente 2
commands2=$(cat <<CMD
PASS $PASSWORD
NICK userB
USER ub 0 * :User B
sleep 2
JOIN #chat
PRIVMSG #chat :Hola UserA! Soy UserB
sleep 2
PRIVMSG #chat :Te escucho claro
sleep 3
CMD
)

expected2=$(cat <<RESP
.*JOIN #chat
.*PRIVMSG #chat :Hola soy UserA
.*PRIVMSG #chat :Alguien me escucha?
RESP
)

run_verified_client "userA" "$commands1" "$expected1" 10 &
run_verified_client "userB" "$commands2" "$expected2" 10 &

wait_and_check 12 "interaccion con verificacion" || ((FAILED_TESTS++))

# PRUEBA 5: Flood suave CON VERIFICACIÓN
echo ""
echo "5️⃣ /$TOTAL_TESTS: FLOOD SUAVE 🌊"
commands=$(cat <<CMD
PASS $PASSWORD
NICK flooder
USER fl 0 * :Flooder
sleep 1
JOIN #flood
PRIVMSG #flood :Mensaje 1
PRIVMSG #flood :Mensaje 2
PRIVMSG #flood :Mensaje 3
PRIVMSG #flood :Mensaje 4
PRIVMSG #flood :Mensaje 5
PRIVMSG #flood :Fin flood
sleep 3
CMD
)

expected_responses=$(cat <<RESP
.*JOIN #flood
.*PRIVMSG #flood :Fin flood
RESP
)

if ! run_verified_client "flooder" "$commands" "$expected_responses" 8; then
    ((RESPONSE_FAILURES++))
fi

wait_and_check 10 "flood con verificacion" || ((FAILED_TESTS++))

# PRUEBA 6: Comando MODE e INVITE CON VERIFICACIÓN
echo ""
echo "6️⃣ /$TOTAL_TESTS: COMANDOS MODE E INVITE ⚙️"
commands=$(cat <<CMD
PASS $PASSWORD
NICK modeuser
USER mu 0 * :Mode User
sleep 1
JOIN #modetest
MODE #modetest
sleep 2
INVITE userA #modetest
sleep 2
CMD
)

expected_responses=$(cat <<RESP
.*JOIN #modetest
.*MODE #modetest
.*INVITE userA
RESP
)

if ! run_verified_client "modeuser" "$commands" "$expected_responses" 6; then
    ((RESPONSE_FAILURES++))
fi

wait_and_check 8 "comandos mode e invite con verificacion" || ((FAILED_TESTS++))
# PRUEBA 7: Comando KICK CON VERIFICACIÓN
echo ""
echo "7️⃣ /$TOTAL_TESTS: COMANDO KICK 🦵"

# Cliente que va a echar
commands1=$(cat <<CMD
PASS $PASSWORD
NICK kicker
USER ki 0 * :Kicker User
sleep 1
JOIN #kickroom
sleep 3
KICK #kickroom kickeduser :Por molesto
sleep 2
CMD
)

expected1=$(cat <<RESP
.*JOIN #kickroom
.*KICK #kickroom kickeduser
RESP
)

# Cliente que será echado
commands2=$(cat <<CMD
PASS $PASSWORD
NICK kickeduser
USER ku 0 * :Kicked User
sleep 2
JOIN #kickroom
PRIVMSG #kickroom :Hola a todos
sleep 5
CMD
)

expected2=$(cat <<RESP
.*JOIN #kickroom
.*KICK #kickroom kickeduser
RESP
)

run_verified_client "kicker" "$commands1" "$expected1" 8 &
run_verified_client "kickeduser" "$commands2" "$expected2" 8 &

wait_and_check 10 "comando kick con verificacion" || ((FAILED_TESTS++))

# PRUEBA 8: Múltiples clientes en mismo canal CON VERIFICACIÓN
echo ""
echo "8️⃣ /$TOTAL_TESTS: MULTIPLES CLIENTES 👥👥👥"
for i in {1..3}; do
    commands=$(cat <<CMD
PASS $PASSWORD
NICK multi$i
USER mu$i 0 * :Multi $i
sleep 2
JOIN #general
PRIVMSG #general :Hola desde multi$i
sleep 3
PRIVMSG #general :Mensaje final de multi$i
sleep 2
CMD
)

    expected_responses=$(cat <<RESP
.*JOIN #general
.*PRIVMSG #general :.*multi
RESP
)

    if ! run_verified_client "multi$i" "$commands" "$expected_responses" 8; then
        ((RESPONSE_FAILURES++))
    fi &
    sleep 0.5
done

wait_and_check 10 "multiples clientes con verificacion" || ((FAILED_TESTS++))

# PRUEBA 9: TEST DE ESTRÉS MÁXIMO
echo ""
echo "9️⃣ /$TOTAL_TESTS: TEST DE ESTRÉS MÁXIMO 💥🔥"

rm -f /tmp/rejected_count_$$ 2>/dev/null
touch /tmp/rejected_count_$$

echo "📈 Fase 1: 80 conexiones base rápidas..."
for i in {1..150}; do
    commands=$(cat <<CMD
PASS $PASSWORD
NICK stress$i
USER st$i 0 * :Stress User $i
sleep 20
CMD
)
    run_silent_client "stress$i" "$commands" 25 &
    if (( i % 10 == 0 )); then
        echo "   ✅ Conectados: $i/80"
        sleep 0.2
    else
        sleep 0.05
    fi
done

echo "📈 Fase 1 completada: 80 conexiones establecidas"
sleep 3
echo ""
echo "🚨 Fase 2: 50 conexiones adicionales (deberían RECHAZARSE)..."
for i in {1..50}; do
    if [ $i -le 5 ]; then
        monitor_rejection "overload$i" "$i" &
    else
        commands=$(cat <<CMD
PASS $PASSWORD
NICK overload$i
USER ov$i 0 * :Overload User $i
sleep 10
CMD
)
        run_silent_client "overload$i" "$commands" 12 &
    fi
    sleep 0.1
done

echo ""
echo "📊 Fase 2 en progreso..."
sleep 10

echo ""
echo "🎯 Fase 3: Verificando estado del servidor..."
if check_server; then
    echo "   ✅ Servidor sigue funcionando después del estrés 👍"
else
    echo "   💥 Servidor crasheó bajo carga 👎"
    ((FAILED_TESTS++))
fi

REJECTED_COUNT=$(wc -l < /tmp/rejected_count_$$ 2>/dev/null || echo "0")
rm -f /tmp/rejected_count_$$ 2>/dev/null

echo ""
echo "📊 RESULTADOS DEL ESTRÉS:"
echo "   🔥 Conexiones rechazadas: $REJECTED_COUNT/5 monitoreadas"

if [ $REJECTED_COUNT -gt 0 ]; then
    echo "   ✅ El servidor RECHAZÓ conexiones (comportamiento esperado) 🎉"
else
    echo "   ⚠️  El servidor NO rechazó conexiones (puede que acepte más de 100)"
fi

wait_and_check 5 "test de estrés máximo" || ((FAILED_TESTS++))

# PRUEBA 10: Respuestas de error esperadas
echo ""
echo "🔟 /$TOTAL_TESTS: RESPUESTAS DE ERROR ESPERADAS 🚫"
commands=$(cat <<CMD
NICK noauth
USER noauth 0 * :No Auth
PRIVMSG nobody :Hola
JOIN #nochannel
KICK #nochannel noauth :Intento
CMD
)

expected_responses=$(cat <<RESP
.*451
.*401
.*403
.*482
RESP
)

if ! run_verified_client "noauth" "$commands" "$expected_responses" 6; then
    ((RESPONSE_FAILURES++))
fi

wait_and_check 6 "respuestas de error esperadas" || ((FAILED_TESTS++))

cleanup

echo ""
echo "========================================"
echo "🎊 PRUEBAS COMPLETADAS 🎊"
echo "📊 RESULTADO GENERAL: $((TOTAL_TESTS - FAILED_TESTS))/$TOTAL_TESTS"
echo "📊 FALLOS DE RESPUESTA: $RESPONSE_FAILURES"

if [ $FAILED_TESTS -eq 0 ] && [ $RESPONSE_FAILURES -eq 0 ]; then
    echo ""
    echo "🎉 ¡TODAS LAS PRUEBAS EXITOSAS! 🎉"
    echo ""
    echo "✅ El servidor manejo correctamente:"
    echo "   🔐 Autenticacion (PASS/NICK/USER) con respuestas"
    echo "   🛠️  Comandos basicos (PING/QUIT) con verificacion"  
    echo "   💬 Canales (JOIN/PART/PRIVMSG) con mensajes"
    echo "   📨 Mensajeria (PRIVMSG) bidireccional"
    echo "   ⚙️  Comandos de canal (TOPIC/MODE/INVITE/KICK)"
    echo "   👥 Multiples clientes simultaneos"
    echo "   🌊 Flood controlado"
    echo "   💥 Estrés máximo (130+ conexiones)"
    echo "   🚫 Manejo correcto de errores IRC"
    echo "   📡 TODAS las respuestas esperadas recibidas"
    echo ""
    echo "🚀 ¡SERVIDOR IRC LISTO PARA PRODUCCIÓN! 🚀"
elif [ $FAILED_TESTS -eq 0 ] && [ $RESPONSE_FAILURES -gt 0 ]; then
    echo ""
    echo "⚠️  Servidor estable pero con respuestas faltantes"
    echo "   ❌ $RESPONSE_FAILURES pruebas tuvieron respuestas incompletas"
    echo "   💡 Revisa los logs para ver qué respuestas faltaron"
else
    echo ""
    echo "❌ $FAILED_TESTS pruebas fallaron + $RESPONSE_FAILURES respuestas faltantes"
    echo "💡 Revisa los logs para más detalles"
fi

echo ""
echo "========================================"

exit $((FAILED_TESTS + RESPONSE_FAILURES))
