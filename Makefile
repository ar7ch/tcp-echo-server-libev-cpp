.PHONY: clean build all docker

PROJECT_BUILD_DIR ?= ./build
SERVER_DEFAULT_PORT ?= 5000

all: clean build

clean:
	rm -rf ./$(PROJECT_BUILD_DIR)

build:
	mkdir -p $(PROJECT_BUILD_DIR)
	cmake -B $(PROJECT_BUILD_DIR) .
	cmake --build $(PROJECT_BUILD_DIR)
	cp $(PROJECT_BUILD_DIR)/compile_commands.json .

run: build
	$(PROJECT_BUILD_DIR)/bin/tcp-echo-server -v $(SERVER_DEFAULT_PORT)

docker:
	docker build -t tcp-echo-server:latest .

tidy:
	find src/ -name '*.cpp' -o -name '*.c' | xargs -P4 -n1 clang-tidy -p build --extra-arg=-std=c++20
