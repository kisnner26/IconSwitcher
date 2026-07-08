# Icon Switcher

Mod de [Geode](https://geode-sdk.org/) para Geometry Dash. Agrega un botón al menú de pausa que abre un popup para cambiar el icono, los colores y el glow del jugador **en tiempo real**, sin reiniciar el nivel.

## Funciones

- Selector de modo (Cubo, Nave, Bola, Ovni, Onda, Robot, Araña, Swing) con vista previa en vivo
- Cambio de ID de icono por modo (flechas `<` `>`)
- Selector de color primario y secundario (color picker nativo de Geode, RGB completo)
- Toggle de Glow
- Los cambios se aplican al instante sobre el jugador en partida (no hace falta pausar y reiniciar)

## Compilar

Requisitos: [CMake](https://cmake.org/), [geode-cli](https://docs.geode-sdk.org/getting-started/), y Geometry Dash instalado.

```sh
geode config setup   # solo la primera vez, vincula tu instalacion de GD
geode build
```

El `.geode` resultante queda en `build/` y se instala automáticamente si el profile está configurado.

## Estructura

- `mod.json` — metadata del mod
- `CMakeLists.txt` — configuración de build
- `src/main.cpp` — hook a `PauseLayer` + popup de selección de icono/colores
