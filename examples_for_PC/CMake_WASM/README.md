# WebAssembly port

# Step

## Install SDL

[follow this](https://wiki.libsdl.org/SDL2/Installation)

## Install Emscripten SDK
[follow this](https://emscripten.org/docs/getting_started/downloads.html)

## Build and Run
1. `mkdir build`
2. `cd build`
3. `emcmake cmake ..`
4. `emmake make`
5. `http-server`
5. open `index.html` in your browser.

## Notes
this is a debug build: `set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -g -s USE_SDL=2")`
It uses the SDL2 library bundled with the Emscripten SDK so I think the `Install SDL``
is not necessary

### Debugging 
I followed the [Debugging WebAssembly with modern tools](https://developer.chrome.com/blog/wasm-debugging-2020/) tutorial and was able to debug at the C source level just fine


Steps:
1. Install Chrome Canary
2. Install the  C/C++ DevTools Support (DWARF) as outline 
3. serve `index.html`  via `http-server` - opening file//wherever/index.html breaks debugging
