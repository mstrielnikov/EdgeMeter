package integration_test

import (
	"testing"

	"go.opentelemetry.io/collector/pdata/pmetric"
)

func validateAnyOTLPMetric(t *testing.T, metrics pmetric.Metrics) bool {
	if metrics.ResourceMetrics().Len() == 0 {
		return false
	}

	valid := false
	for i := 0; i < metrics.ResourceMetrics().Len(); i++ {
		rm := metrics.ResourceMetrics().At(i)
		for j := 0; j < rm.ScopeMetrics().Len(); j++ {
			sm := rm.ScopeMetrics().At(j)
			for k := 0; k < sm.Metrics().Len(); k++ {
				m := sm.Metrics().At(k)
				t.Logf("Decoded Canonical OTLP Metric: %s (Type: %v)", m.Name(), m.Type())
				
				// Verify DataPoints serialization mapping correctly binds data
				switch m.Type() {
				case pmetric.MetricTypeGauge:
					if m.Gauge().DataPoints().Len() > 0 {
						dp := m.Gauge().DataPoints().At(0)
						t.Logf(" -> Gauge Value: %v", dp.DoubleValue())
						valid = true
					}
				case pmetric.MetricTypeSum:
					if m.Sum().DataPoints().Len() > 0 {
						dp := m.Sum().DataPoints().At(0)
						t.Logf(" -> Sum Value: %v", dp.DoubleValue())
						valid = true
					}
				case pmetric.MetricTypeHistogram:
				case pmetric.MetricTypeSummary:
				}
			}
		}
	}

	return valid
}
