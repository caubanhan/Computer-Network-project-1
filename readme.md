# Computer Network Project 1

Simple client/server network project (C++17, CMake + Ninja).  
This repository contains a networked server and a client implementation built with CMake (Ninja generator) and tested with Visual Studio 2022.

## Contents
- `server/` server implementation (sources: `Server.h`, `Server.cpp`, `ServerWorker.h`, `ServerWorker.cpp`, `main.cpp`)
- `client/` client implementation (sources: `Client.h`, `Client.cpp`, `main_client.cpp`)
- `CMakeLists.txt` and build configuration at repository root

## Goals
- Provide a minimal, maintainable client/server example using modern C++ (C++17).
- CMake-based cross-platform build with the Ninja generator.
- Easy to build and run from both CLI and Visual Studio 2022.

## Requirements
- CMake >= 3.16 (project tested with `3.31.6-msvc6`)
- Ninja build system
- Visual Studio 2022 (recommended) or another modern C++ toolchain
- C++17-compatible compiler
- Compiler GCC MinGW64 (15.2.0 recommeneded)

## Build (Command Line)
Recommended workflow (from repository root): <br>
NOTE !!: only for Windows

1. build server
```bash
cd ./server
make all
```

2. build client
```bash
cd ./client
make all
```

## Build (Visual Studio 2022)
1. Open the repository in Visual Studio: use __File > Open > Folder__ and select the project root.
2. Visual Studio will detect `CMakeLists.txt`. Use the CMake menu or toolbar to __CMake: Configure__ and then __CMake: Build__.
3. Select the CMake target for `server` or `client` from the CMake Targets view and run.

## Run
- Default server port: `8089` (see `server/main.cpp`, `#define DEFAULT_PORT 8089`).
- Example: Start the server (from a shell or VS debugger):
  - Windows (from build output folder): `server.exe <server-port>`
- Run the client and point it at the server:
  - `client_app.exe <server-host-or-ip> <server-port> <file-name>`
  - The example client sends a simple `SETUP` RTSP message and listens for the server reply.

Example (localhost):
1. Start server:
   - `cd server/`
   - `server.exe 8089`
2. In a second terminal start client:
   - `cd client/`
   - `client.exe 127.0.0.1 8089 movie.mjpeg`

Notes:
- The example client sends `SETUP movie.Mjpeg RTSP/1.0` with `client_port=25000`. The server opens an RTP UDP socket and uses the supplied client port for RTP packets.
- The server code contains RTSP methods: `SETUP`, `PLAY`, `PAUSE`, `TEARDOWN`. RTP packetization is handled in `ServerWorker::sendRtp()`.

## Troubleshooting
- Firewall: allow UDP/TCP ports used by server (default RTSP TCP 554 and RTP UDP client port such as 25000).
- Ports <1024 may require elevated permissions on some systems.
- If builds fail, confirm you have Ninja installed and that `cmake` on PATH is the required version.

## File layout
- `server/` RTSP server code and worker threads
- `client/` simple RTSP client examples
- `common/` shared helpers (e.g., `RtpPacket.h`) (if present in repo)

## Contributing
- Open PRs for fixes/features.
- Keep changes focused and add short notes to the PR describing behavior.

## License
No license file included. Add a `LICENSE` file to document allowed reuse (e.g., MIT).

---

If you want, I can:
- Produce a short example showing an RTSP exchange (raw request/response),
- Add a simple `CMakePresets.json` for consistent CLI/VS builds,
- Or update this README to include exact binary paths after your current CMake configuration.