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
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	successCh := make(chan bool, 1)

	mux := http.NewServeMux()
	mux.HandleFunc("/v1/metrics", func(w http.ResponseWriter, r *http.Request) {
		body, err := io.ReadAll(r.Body)
		if err != nil {
			t.Logf("Plugin test: failed to read body: %v", err)
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		t.Logf("Plugin test: received %d bytes", len(body))

		metrics, err := new(pmetric.JSONUnmarshaler).UnmarshalMetrics(body)
		if err != nil {
			t.Logf("Plugin test: JSON unmarshal error: %v (raw: %s)", err, string(body))
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		for i := 0; i < metrics.ResourceMetrics().Len(); i++ {
			rm := metrics.ResourceMetrics().At(i)
			for j := 0; j < rm.ScopeMetrics().Len(); j++ {
				sm := rm.ScopeMetrics().At(j)
				for k := 0; k < sm.Metrics().Len(); k++ {
					m := sm.Metrics().At(k)
					t.Logf("Plugin test: metric=%s type=%v", m.Name(), m.Type())
					if m.Name() == "plugin.external.mock" && m.Type() == pmetric.MetricTypeGauge {
						if m.Gauge().DataPoints().Len() > 0 {
							dp := m.Gauge().DataPoints().At(0)
							t.Logf("Plugin test: gauge value=%v", dp.DoubleValue())
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
		w.WriteHeader(http.StatusOK)
	})

	server := &http.Server{Addr: "localhost:4318", Handler: mux}
	go server.ListenAndServe()
	defer server.Close()

	// Allow HTTP server goroutine to bind before agent retries
	time.Sleep(100 * time.Millisecond)

	select {
	case <-ctx.Done():
		t.Fatalf("Timeout: no plugin.external.mock metric received on :4318/v1/metrics")
	case <-successCh:
		t.Logf("✓ Verified plugin.external.mock gauge=99.9 deserialized from C ABI plugin")
	}
}
