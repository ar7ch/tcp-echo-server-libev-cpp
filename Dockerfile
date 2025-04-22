# stage 1: build
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and dependencies
RUN apt-get update && apt-get install -y \
    git \
    cmake \
    build-essential \
	libev4 \
    libev-dev \
	pkg-config \
    make \
    && rm -rf /var/lib/apt/lists/*

# Copy project files
WORKDIR /src
COPY . .

# Build using Makefile target
RUN make build

# stage 2: runtime
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install only runtime dependencies
RUN apt-get update && apt-get install -y \
    libev4 \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Copy the built binary from the builder stage
COPY --from=builder /src/build/bin/tcp-echo-server /usr/local/bin/tcp-echo-server

# Set default command
CMD ["tcp-echo-server", "5000", "-v"]
