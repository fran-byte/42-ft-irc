```c++ name=src/main.cpp
#include <iostream>
#include <cstdlib>
#include "Server.hpp"

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
}
```

### Explicación detallada de `main.cpp`

1. **Includes**:

   - `<iostream>`: Para entrada/salida estándar.
   - `<cstdlib>`: Para usar `std::atoi` (convertir string a entero).
   - `"Server.hpp"`: Incluye la definición de la clase `Server`, el corazón del servidor IRC.

2. **Función principal**:
   - `int main(int argc, char** argv)`: Punto de entrada del programa.
   - **Chequeo de argumentos**:
     - Espera exactamente 2 argumentos: el puerto y la contraseña para el servidor IRC.
     - Si no se pasan, muestra el uso correcto y termina con error.
   - **Interpretación de argumentos**:
     - `port` se convierte del primer argumento a un `unsigned short`.
     - `password` toma el segundo argumento.
   - **Ejecución del servidor**:
     - Se crea un objeto `Server` con el puerto y contraseña dados.
     - Se llama a `srv.run()`, que arranca el bucle principal del servidor (acepta conexiones, procesa mensajes, etc.).
   - **Manejo de errores**:
     - Si ocurre cualquier excepción, se muestra el error por salida estándar y termina con error.

### Resumen

- El `main.cpp` valida los argumentos (puerto y contraseña), crea el servidor y lo ejecuta.
- Toda la lógica principal del servidor está encapsulada en la clase `Server`.
- Si hay algún fallo (por ejemplo, puerto inválido, error en la red, etc.), el programa termina mostrando el motivo.
