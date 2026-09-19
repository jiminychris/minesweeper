clang -std=c99 --target=wasm32 -nostdlib -Wl,--import-memory -Wl,--allow-undefined -Wl,--no-entry -Wl,--export=GameUpdateAndRender -Wl,--export=__heap_base -o minesweeper.wasm minesweeper.c
