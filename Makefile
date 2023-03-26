# EdgeMeter Makefile

CXX ?= g++
CXXFLAGS = -std=c++2a -Wall -Wextra -pthread
LDFLAGS = -pthread

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Vcpkg Dependency Isolation
VCPKG_ROOT ?= ./.vcpkg
VCPKG_EXEC = $(VCPKG_ROOT)/vcpkg
VCPKG_TRIPLET = x64-linux

# Provide default include and lib paths based on vcpkg installation
INCLUDES = -I$(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)/include
LIBS = -L$(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)/lib

# Feature Flags
USE_TLS ?= 1
USE_GRPC ?= 1
USE_WEBSOCKETS ?= 1
USE_NATS ?= 0

# WASM Override defaults: Emscripten doesn't natively handle gRPC or native openSSL without complex wrappers
ifeq ($(CXX), emcc)
    USE_GRPC = 0
    USE_TLS = 0
    # NATS POSIX TCP sockets inherently bridge across Emscripten natively without native TLS!
    USE_WEBSOCKETS = 1
    VCPKG_TRIPLET = wasm32-emscripten
    CXXFLAGS += -s WASM=1 -s ASYNCIFY=1
endif

ifeq ($(USE_TLS), 1)
    CXXFLAGS += -DUSE_TLS
    LIBS += -lssl -lcrypto
endif

ifeq ($(USE_GRPC), 1)
    CXXFLAGS += -DUSE_GRPC
endif

ifeq ($(USE_WEBSOCKETS), 1)
    CXXFLAGS += -DUSE_WEBSOCKETS
endif

ifeq ($(USE_NATS), 1)
    CXXFLAGS += -DUSE_NATS
endif

# Source Files
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TARGET = $(BIN_DIR)/EdgeMeter

.PHONY: all clean vcpkg_bootstrap certs service test_ws test_ws_tls test_grpc test_grpc_tls test_nats test_nats_tls test

all: $(TARGET)

certs:
	@mkdir -p certs
	openssl req -x509 -newkey rsa:4096 -keyout certs/server.key -out certs/server.crt -days 365 -nodes -subj "/CN=localhost"

service:
	@echo "Installing TelemetryAgent.service to /etc/systemd/system/"
	cp TelemetryAgent.service /etc/systemd/system/
	systemctl daemon-reload

test_ws:
	@echo "=== Testing WebSockets natively ==="
	fuser -k 8080/tcp 2>/dev/null || true
	fuser -k 4317/tcp 2>/dev/null || true
	fuser -k 4222/tcp 2>/dev/null || true
	killall -q EdgeMeter || true
	$(MAKE) clean
	$(MAKE) USE_WEBSOCKETS=1 USE_GRPC=0 USE_NATS=0 USE_TLS=0 -j4
	./bin/EdgeMeter & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestWebSockets_NoTLS; \
	TEST_RES=$$?; \
	kill $$AGENT_PID || true; \
	wait $$AGENT_PID 2>/dev/null || true; \
	exit $$TEST_RES

test_ws_tls:
	@echo "=== Testing WebSockets TLS natively ==="
	fuser -k 8080/tcp 2>/dev/null || true
	fuser -k 4317/tcp 2>/dev/null || true
	fuser -k 4222/tcp 2>/dev/null || true
	killall -q EdgeMeter || true
	$(MAKE) clean
	$(MAKE) USE_WEBSOCKETS=1 USE_GRPC=0 USE_NATS=0 USE_TLS=1 -j4
	./bin/EdgeMeter & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestWebSockets_TLS; \
	TEST_RES=$$?; \
	kill $$AGENT_PID || true; \
	wait $$AGENT_PID 2>/dev/null || true; \
	exit $$TEST_RES

test_grpc:
	@echo "=== Testing gRPC HTTP natively ==="
	fuser -k 8080/tcp 2>/dev/null || true
	fuser -k 4317/tcp 2>/dev/null || true
	fuser -k 4222/tcp 2>/dev/null || true
	killall -q EdgeMeter || true
	$(MAKE) clean
	$(MAKE) USE_WEBSOCKETS=0 USE_GRPC=1 USE_NATS=0 USE_TLS=0 -j4
	./bin/EdgeMeter & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestGRPC_NoTLS; \
	TEST_RES=$$?; \
	kill $$AGENT_PID || true; \
	wait $$AGENT_PID 2>/dev/null || true; \
	exit $$TEST_RES

test_grpc_tls:
	@echo "=== Testing gRPC TLS natively ==="
	fuser -k 8080/tcp 2>/dev/null || true
	fuser -k 4317/tcp 2>/dev/null || true
	fuser -k 4222/tcp 2>/dev/null || true
	killall -q EdgeMeter || true
	$(MAKE) clean
	$(MAKE) USE_WEBSOCKETS=0 USE_GRPC=1 USE_NATS=0 USE_TLS=1 -j4
	./bin/EdgeMeter & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestGRPC_TLS; \
	TEST_RES=$$?; \
	kill $$AGENT_PID || true; \
	wait $$AGENT_PID 2>/dev/null || true; \
	exit $$TEST_RES

test_nats:
	@echo "=== Testing NATS TCP natively ==="
	fuser -k 8080/tcp 2>/dev/null || true
	fuser -k 4317/tcp 2>/dev/null || true
	fuser -k 4222/tcp 2>/dev/null || true
	killall -q EdgeMeter || true
	$(MAKE) clean
	$(MAKE) USE_WEBSOCKETS=0 USE_GRPC=0 USE_NATS=1 USE_TLS=0 -j4
	./bin/EdgeMeter & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestNATS_NoTLS; \
	TEST_RES=$$?; \
	kill $$AGENT_PID || true; \
	wait $$AGENT_PID 2>/dev/null || true; \
	exit $$TEST_RES

test_nats_tls:
	@echo "=== Testing NATS TLS natively ==="
	fuser -k 8080/tcp 2>/dev/null || true
	fuser -k 4317/tcp 2>/dev/null || true
	fuser -k 4222/tcp 2>/dev/null || true
	killall -q EdgeMeter || true
	$(MAKE) clean
	$(MAKE) USE_WEBSOCKETS=0 USE_GRPC=0 USE_NATS=1 USE_TLS=1 -j4
	./bin/EdgeMeter & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestNATS_TLS; \
	TEST_RES=$$?; \
	kill $$AGENT_PID || true; \
	wait $$AGENT_PID 2>/dev/null || true; \
	exit $$TEST_RES

test: test_ws test_ws_tls test_grpc test_grpc_tls test_nats test_nats_tls
	@echo "=== ✓ All Component Integration Tests Passed Matrix Identically ==="

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) -o $@ $(INCLUDES) $(LIBS) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

vcpkg_bootstrap:
	if [ ! -d "$(VCPKG_ROOT)" ]; then \
		git clone https://github.com/microsoft/vcpkg.git $(VCPKG_ROOT) && \
		$(VCPKG_ROOT)/bootstrap-vcpkg.sh -disableMetrics; \
	fi
	$(VCPKG_EXEC) install inja nlohmann-json --triplet $(VCPKG_TRIPLET)
	@echo "Notice: Execute vcpkg_grpc manually to compile full opentelemetry bindings."

vcpkg_grpc:
	$(VCPKG_EXEC) install opentelemetry-cpp[grpc,metrics] openssl --triplet $(VCPKG_TRIPLET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
