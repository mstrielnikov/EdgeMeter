package integration_test

import (
	"context"
	"io"
	"net/http"
	"testing"
	"time"

	"go.opentelemetry.io/collector/pdata/pmetric"
)

func TestPluginPipeline(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	successCh := make(chan bool, 1)

	// Intercept the HTTP requests matching the generic exporter sink parameters
	mux := http.NewServeMux()
	mux.HandleFunc("/v1/metrics", func(w http.ResponseWriter, r *http.Request) {
		body, err := io.ReadAll(r.Body)
		if err == nil {
			metrics, err2 := new(pmetric.JSONUnmarshaler).UnmarshalMetrics(body)
			if err2 == nil {
				// We iterate looking explicitly for our mocked C ABI metric injection
				for i := 0; i < metrics.ResourceMetrics().Len(); i++ {
					rm := metrics.ResourceMetrics().At(i)
					for j := 0; j < rm.ScopeMetrics().Len(); j++ {
						sm := rm.ScopeMetrics().At(j)
						for k := 0; k < sm.Metrics().Len(); k++ {
							m := sm.Metrics().At(k)
							if m.Name() == "plugin.external.mock" {
								if m.Gauge().DataPoints().Len() > 0 {
									dp := m.Gauge().DataPoints().At(0)
									if dp.DoubleValue() == 99.9 {
										select {
										case successCh <- true:
										default:
										}
									}
								}
							}
						}
					}
				}
			}
		}
		w.WriteHeader(http.StatusOK)
	})

	server := &http.Server{Addr: "localhost:4318", Handler: mux}
	go server.ListenAndServe()
	defer server.Close()

	select {
	case <-ctx.Done():
		t.Fatalf("Timeout: Target mock plugin telemetry pipeline completely failed resolving metrics.")
	case <-successCh:
		t.Logf("✓ Verified exact mock plugin payload structural typestates deserialized accurately!")
	}
}
