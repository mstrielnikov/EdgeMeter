package integration_test

import (
	"context"
	"net/http"
	"testing"
	"time"

	"github.com/coder/websocket"
	"go.opentelemetry.io/collector/pdata/pmetric"
)

func runWSTest(t *testing.T, expectedTLS bool, makeFlags []string) {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	successCh := make(chan bool, 1)

	wsMux := http.NewServeMux()
	wsMux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		t.Logf("WS: Incoming connection request intercepted...")
		conn, err := websocket.Accept(w, r, &websocket.AcceptOptions{InsecureSkipVerify: true})
		if err != nil {
			t.Logf("WS: Accept error: %v", err)
			return
		}
		t.Logf("WS: Connection properly accepted securely natively!")
		defer conn.Close(websocket.StatusNormalClosure, "")

		for {
			typ, msg, err := conn.Read(ctx)
			if err != nil {
				break
			}
			if typ != websocket.MessageText {
				continue
			}

			metrics, err := new(pmetric.JSONUnmarshaler).UnmarshalMetrics(msg)
			if err == nil && metrics.ResourceMetrics().Len() > 0 {
				select {
				case successCh <- true:
				default:
				}
			}
		}
	})

	server := &http.Server{Addr: "localhost:8080", Handler: wsMux}
	if expectedTLS {
		go server.ListenAndServeTLS("../../certs/server.crt", "../../certs/server.key")
	} else {
		go server.ListenAndServe()
	}
	defer server.Close()

	time.Sleep(1 * time.Second)

	select {
	case <-ctx.Done():
		t.Fatalf("Timeout: WebSockets failed structurally asserting payloads universally!")
	case <-successCh:
		t.Logf("✓ Verified explicit WebSocket OTLP Native Metrics (TLS: %v)", expectedTLS)
	}
}

func TestWebSockets_NoTLS(t *testing.T) {
	runWSTest(t, false, []string{"USE_WEBSOCKETS=1", "USE_GRPC=0", "USE_NATS=0", "USE_TLS=0"})
}

func TestWebSockets_TLS(t *testing.T) {
	runWSTest(t, true, []string{"USE_WEBSOCKETS=1", "USE_GRPC=0", "USE_NATS=0", "USE_TLS=1"})
}
