# TCP echo server
[![Build and Push Docker Image](https://github.com/ar7ch/tcp-echo-server-libev-cpp/actions/workflows/docker-build.yml/badge.svg)](https://github.com/ar7ch/tcp-echo-server-libev-cpp/actions/workflows/docker-build.yml)

Async libev-based TCP server that simply echoes everything that clients send.

## Dependencies
- `C++20`-capable compiler
- `make` of any version - for convenience shortcuts
- `cmake` 3.21 - build system
- `libev` v4.33 - event loop (most Linux distros have `libev-dev` for building, `libev` just for usage) - **need to be present on the system**
- `spdlog` v1.15.2 (auto-fetched by cmake) - for logging
- `CLI11` v2.5.0 (auto-fetched by cmake) - for parsing command-line arguments

You can load a Nix flake with all build dependencies using `nix develop`.

## Build

### Manually

Ensure that you have all the build dependencies installed.

```
make build
```

### Docker

**Option 1**:
1. build container with `make docker` (or, the same, `docker build -t tcp-echo-server:latest .`)
2. launch it with `docker run -it tcp-echo-server:latest`
    (default listening port is 5000, don't forget about port forwarding)

**Option 2**:
1. use CI-built docker image on DockerHub: [ar7ch/tcp-echo-server](https://hub.docker.com/r/ar7ch/tcp-echo-server)

```bash
docker run -it ar7ch/tcp-echo-server
```

## Run

The binary `tcp-echo-server` will be located in `./build/bin` (or in `/usr/local/bin` in Docker image)

Usage:
```bash
./tcp-echo-server <listen port> [-v]
```
- `-v` enables debug logs.

## Client
This project also ships a simple async Python client.

Usage:
```bash
./client.py <server ip> <server port>
```

You will have an interactive session where you can type input to send to server and receive responses.

- Moreover, there is a **one-shot mode** `-o` where you can send message non-interactively.

```bash
./client.py <server ip> <server port> -o "hello world"
```

In one-shot mode, the client will exit after receiving a response.
