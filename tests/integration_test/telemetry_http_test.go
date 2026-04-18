package integration_test

import (
	"context"
	"io"
	"net/http"
	"testing"
	"time"

	"go.opentelemetry.io/collector/pdata/pmetric"
)

func runOtelHttpTest(t *testing.T, expectedTLS bool) {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	successCh := make(chan bool, 1)

	otlpMux := http.NewServeMux()
	otlpMux.HandleFunc("/v1/metrics", func(w http.ResponseWriter, r *http.Request) {
		t.Logf("HTTP: Connection request handled actively securely")
		body, _ := io.ReadAll(r.Body)
		metrics, err := new(pmetric.JSONUnmarshaler).UnmarshalMetrics(body)
		if err == nil && validateAnyOTLPMetric(t, metrics) {
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
		t.Fatalf("Timeout: HTTP OTLP natively failed structurally asserting payloads!")
	case <-successCh:
		t.Logf("✓ Verified explicit HTTP OTLP Native Metrics (TLS: %v)", expectedTLS)
	}
}

func TestHttp_NoTLS(t *testing.T) {
	runOtelHttpTest(t, false)
}

func TestHttp_TLS(t *testing.T) {
	runOtelHttpTest(t, true)
}
