# Computer Network Project 1

Simple client/server network project (C++17, CMake + Ninja).  
This repository contains a networked server and a client implementation built with CMake (Ninja generator) and tested with Visual Studio 2022.

## Contents
- `server/` — server implementation (sources: `Server.h`, `Server.cpp`, `ServerWorker.h`, `ServerWorker.cpp`, `main.cpp`)
- `client/` — client implementation (sources: `Client.h`, `Client.cpp`, `main_client.cpp`)
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

## Build (Command Line)
Recommended workflow (from repository root):

1. Create a build directory: