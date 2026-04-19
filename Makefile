# EdgeMeter SDK — Makefile
#
# SDK library objects are compiled from src/ (excluding nothing — there is no main.cpp).
# Each example is compiled separately and linked with the SDK objects.
#
# Usage:
#   make http_otlp            # Build HTTP/OTLP agent (TLS off by default)
#   make http_otlp USE_TLS=1  # Build HTTP/OTLP agent with TLS
#   make ws_otlp              # Build WebSocket/OTLP agent
#   make ws_otlp USE_TLS=1
#   make nats_otlp            # Build NATS/OTLP agent
#   make nats_otlp USE_TLS=1
#   make all                  # Build all three agents (no TLS)
#   make test                 # Run full integration matrix

CXX      ?= g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pthread
LDFLAGS  = -pthread

# Directories
SRC_DIR  = src
OBJ_DIR  = obj
BIN_DIR  = bin
EX_DIR   = examples

# Vcpkg
VCPKG_ROOT    ?= ./.vcpkg
VCPKG_TRIPLET ?= x64-linux
INCLUDES = -Iinclude -I$(SRC_DIR) -I$(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)/include
LIBS     = -L$(VCPKG_ROOT)/installed/$(VCPKG_TRIPLET)/lib

# TLS toggle — exposed strictly for example binaries
USE_TLS ?= 0
ifeq ($(USE_TLS), 1)
    CXXFLAGS += -DUSE_TLS
endif

# Always link OpenSSL natively because the SDK library compiled components
# are now structurally decoupled and permanently retain Secure TLS capabilities.
LIBS += -lssl -lcrypto

# --- SDK library objects (all .cpp under src/) ---
LIB_SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
LIB_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/sdk/%.o,$(LIB_SRCS))

$(OBJ_DIR)/sdk/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Example helpers ---
define BUILD_EXAMPLE
$(BIN_DIR)/$(1): $(LIB_OBJS) $(EX_DIR)/$(2)/main.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(EX_DIR)/$(2)/main.cpp $(LIB_OBJS) -o $$@ $(LIBS) $(LDFLAGS)
endef

$(eval $(call BUILD_EXAMPLE,http_otlp_agent,http_otlp))
$(eval $(call BUILD_EXAMPLE,ws_otlp_agent,websocket_otlp))
$(eval $(call BUILD_EXAMPLE,nats_otlp_agent,nats_otlp))

# Convenience aliases
http_otlp:  $(BIN_DIR)/http_otlp_agent
ws_otlp:    $(BIN_DIR)/ws_otlp_agent
nats_otlp:  $(BIN_DIR)/nats_otlp_agent

all: http_otlp ws_otlp nats_otlp

.PHONY: all http_otlp ws_otlp nats_otlp clean certs service \
        test_ws test_ws_tls test_grpc test_grpc_tls test_nats test_nats_tls test

# --- Certificates ---
certs:
	@mkdir -p certs
	openssl req -x509 -newkey rsa:4096 -keyout certs/server.key -out certs/server.crt \
	    -days 365 -nodes -subj "/CN=localhost"

# --- Systemd service ---
service:
	@echo "Installing EdgeMeter.service to /etc/systemd/system/"
	cp EdgeMeter.service /etc/systemd/system/
	systemctl daemon-reload

# ─── Integration tests ───────────────────────────────────────────────────────
# Each test target builds the relevant agent binary for the right TLS flag,
# launches it, runs the Go integration test, then tears it down.

define RUN_TEST
	fuser -k 8080/tcp 2>/dev/null || true
	fuser -k 4317/tcp 2>/dev/null || true
	fuser -k 4222/tcp 2>/dev/null || true
	killall -q http_otlp_agent ws_otlp_agent nats_otlp_agent 2>/dev/null || true
endef

test_ws:
	@echo "=== Testing WebSockets natively ==="
	$(MAKE) clean
	$(MAKE) ws_otlp USE_TLS=0
	./bin/ws_otlp_agent & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestWebSockets_NoTLS; \
	TEST_RES=$$?; kill $$AGENT_PID || true; wait $$AGENT_PID 2>/dev/null || true; exit $$TEST_RES

test_ws_tls:
	@echo "=== Testing WebSockets TLS natively ==="
	$(MAKE) clean
	$(MAKE) ws_otlp USE_TLS=1
	./bin/ws_otlp_agent & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestWebSockets_TLS; \
	TEST_RES=$$?; kill $$AGENT_PID || true; wait $$AGENT_PID 2>/dev/null || true; exit $$TEST_RES

test_grpc:
	@echo "=== Testing HTTP/OTLP natively ==="
	$(MAKE) clean
	$(MAKE) http_otlp USE_TLS=0
	./bin/http_otlp_agent & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestHttp_NoTLS; \
	TEST_RES=$$?; kill $$AGENT_PID || true; wait $$AGENT_PID 2>/dev/null || true; exit $$TEST_RES

test_grpc_tls:
	@echo "=== Testing HTTP/OTLP TLS natively ==="
	$(MAKE) clean
	$(MAKE) http_otlp USE_TLS=1
	./bin/http_otlp_agent & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestHttp_TLS; \
	TEST_RES=$$?; kill $$AGENT_PID || true; wait $$AGENT_PID 2>/dev/null || true; exit $$TEST_RES

test_nats:
	@echo "=== Testing NATS TCP natively ==="
	$(MAKE) clean
	$(MAKE) nats_otlp USE_TLS=0
	./bin/nats_otlp_agent & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestNATS_NoTLS; \
	TEST_RES=$$?; kill $$AGENT_PID || true; wait $$AGENT_PID 2>/dev/null || true; exit $$TEST_RES

test_nats_tls:
	@echo "=== Testing NATS TLS natively ==="
	$(MAKE) clean
	$(MAKE) nats_otlp USE_TLS=1
	./bin/nats_otlp_agent & AGENT_PID=$$!; \
	sleep 2; \
	cd tests/integration_test && go test -v -run TestNATS_TLS; \
	TEST_RES=$$?; kill $$AGENT_PID || true; wait $$AGENT_PID 2>/dev/null || true; exit $$TEST_RES

test: test_ws test_ws_tls test_grpc test_grpc_tls test_nats test_nats_tls
	@echo "=== ✓ All Component Integration Tests Passed Matrix Identically ==="

# --- Vcpkg bootstrap ---
vcpkg_bootstrap:
	if [ ! -d "$(VCPKG_ROOT)" ]; then \
		git clone https://github.com/microsoft/vcpkg.git $(VCPKG_ROOT) && \
		$(VCPKG_ROOT)/bootstrap-vcpkg.sh -disableMetrics; \
	fi
	$(VCPKG_ROOT)/vcpkg install inja nlohmann-json --triplet $(VCPKG_TRIPLET)

vcpkg_grpc:
	$(VCPKG_ROOT)/vcpkg install opentelemetry-cpp[grpc,metrics] openssl --triplet $(VCPKG_TRIPLET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
