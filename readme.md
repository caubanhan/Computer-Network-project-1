# RTSP Video Streaming Client/Server

A C++17 RTSP server and client for real-time video streaming over RTP/UDP.

## Quick Start

### Build
```bash
mkdir build && cd build
cmake -G Ninja ..
ninja
```

### Run
**Terminal 1 (Server):**
```bash
build\server\server.exe
```

**Terminal 2 (Client):**
```bash
build\client\client.exe
```

Use the client window buttons: SETUP → PLAY → PAUSE/TEARDOWN

## Components
- **Server**: Streams MJPEG video over RTSP (TCP 554) with RTP/UDP (25000)
- **Client**: Receives, decodes, and displays video with interactive UI
- **Common**: RTP packet handling and H.264 packetization

## Requirements
- CMake 3.16+, Ninja, Visual Studio 2022
- C++17 compiler
- SDL2 (included in `client/lib/` and `client/include/`)

## Customizing SDL2 Library Paths

If you need to use a different SDL2 installation, edit **`client/CMakeLists.txt`** lines 17–18:

```cmake
file(TO_CMAKE_PATH "D:/HCMUS/2025-2026/Computer Network/Code/client/lib" SDL_LIB)
file(TO_CMAKE_PATH "D:/HCMUS/2025-2026/Computer Network/Code/client/include" SDL_INC)
```

Replace with your SDL2 paths:
```cmake
file(TO_CMAKE_PATH "C:/your/path/to/SDL2/lib" SDL_LIB)
file(TO_CMAKE_PATH "C:/your/path/to/SDL2/include" SDL_INC)
```

Or use system SDL2 (if installed globally):
```cmake
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
target_link_libraries(client_app PRIVATE SDL2::SDL2 SDL2::SDL2_ttf)
```

---

If you want, I can:
- Produce a short example showing an RTSP exchange (raw request/response),
- Add a simple `CMakePresets.json` for consistent CLI/VS builds,
- Or update this README to include exact binary paths after your current CMake configuration.