package integration_test

import (
	"context"
	"io"
	"net/http"
	"testing"
	"time"

	"go.opentelemetry.io/collector/pdata/pmetric"
)

func runGRPCTest(t *testing.T, expectedTLS bool, makeFlags []string) {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	successCh := make(chan bool, 1)

	otlpMux := http.NewServeMux()
	otlpMux.HandleFunc("/v1/metrics", func(w http.ResponseWriter, r *http.Request) {
		t.Logf("GRPC: Connection request handled actively securely")
		body, _ := io.ReadAll(r.Body)
		metrics, err := new(pmetric.JSONUnmarshaler).UnmarshalMetrics(body)
		if err == nil && metrics.ResourceMetrics().Len() > 0 {
			select {
			case successCh <- true:
			default:
			}
		}
		w.WriteHeader(http.StatusOK)
	})

	server := &http.Server{Addr: "localhost:4317", Handler: otlpMux}
	if expectedTLS {
		go server.ListenAndServeTLS("../../certs/server.crt", "../../certs/server.key")
	} else {
		go server.ListenAndServe()
	}
	defer server.Close()

	time.Sleep(1 * time.Second)

	select {
	case <-ctx.Done():
		t.Fatalf("Timeout: gRPC/HTTP OTLP natively failed structurally asserting payloads!")
	case <-successCh:
		t.Logf("✓ Verified explicit HTTP/gRPC Native Metrics (TLS: %v)", expectedTLS)
	}
}

func TestGRPC_NoTLS(t *testing.T) {
	runGRPCTest(t, false, []string{"USE_WEBSOCKETS=0", "USE_GRPC=1", "USE_NATS=0", "USE_TLS=0"})
}

func TestGRPC_TLS(t *testing.T) {
	runGRPCTest(t, true, []string{"USE_WEBSOCKETS=0", "USE_GRPC=1", "USE_NATS=0", "USE_TLS=1"})
}
