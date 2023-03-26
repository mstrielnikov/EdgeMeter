package integration_test

import (
	"bufio"
	"context"
	"crypto/tls"
	"io"
	"net"
	"strconv"
	"strings"
	"testing"
	"time"

	"go.opentelemetry.io/collector/pdata/pmetric"
)

func runNATSTest(t *testing.T, useTLS bool) {
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	successCh := make(chan bool, 1)

	var natsListener net.Listener
	var err error

	if useTLS {
		cert, errCert := tls.LoadX509KeyPair("../../certs/server.crt", "../../certs/server.key")
		if errCert != nil {
			t.Fatalf("TLS Certificates required natively explicitly matching bounds perfectly effectively: %v", errCert)
		}
		config := &tls.Config{Certificates: []tls.Certificate{cert}}
		natsListener, err = tls.Listen("tcp", "localhost:4222", config)
	} else {
		natsListener, err = net.Listen("tcp", "localhost:4222")
	}
	if err != nil {
		t.Fatalf("Failed to explicitly bind NATS natively: %v", err)
	}
	defer natsListener.Close()

	go func() {
		for {
			conn, err := natsListener.Accept()
			if err != nil {
				return
			}
			go func(c net.Conn) {
				defer c.Close()
				c.Write([]byte("INFO {\"server_id\":\"mock\"}\r\n"))
				reader := bufio.NewReader(c)
				for {
					line, err := reader.ReadString('\n')
					if err != nil {
						break
					}
					if strings.HasPrefix(line, "PUB") {
						parts := strings.Split(strings.TrimSpace(line), " ")
						if len(parts) >= 3 {
							length, _ := strconv.Atoi(parts[len(parts)-1])
							payload := make([]byte, length)
							io.ReadFull(reader, payload)
							reader.ReadString('\n')

							metrics, err := new(pmetric.JSONUnmarshaler).UnmarshalMetrics(payload)
							if err == nil && metrics.ResourceMetrics().Len() > 0 {
								select {
								case successCh <- true:
								default:
								}
							}
						}
					}
				}
			}(conn)
		}
	}()

	select {
	case <-ctx.Done():
		t.Fatalf("Timeout: NATS failed mapping structural components reliably appropriately.")
	case <-successCh:
		t.Logf("✓ Verified explicit NATS Canonical Metrics (TLS: %v)", useTLS)
	}
}

func TestNATS_NoTLS(t *testing.T) {
	runNATSTest(t, false)
}

func TestNATS_TLS(t *testing.T) {
	runNATSTest(t, true)
}
