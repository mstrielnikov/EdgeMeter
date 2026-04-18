#ifndef OTLP_TEMPLATE_HPP
#define OTLP_TEMPLATE_HPP

#include <string>
#include <string_view>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <edgemeter/core/Config.hpp>
#include <span>
#include <edgemeter/telemetry/SystemMetrics.hpp>

namespace telemetry {

constexpr const char* otlp_json_template = R"({
  "resourceMetrics": [
    {
      "resource": {
        "attributes": [
          { "key": "service.name", "value": { "stringValue": "{{ program_name }}" } },
          { "key": "host.name", "value": { "stringValue": "{{ hostname }}" } },
          { "key": "process.runtime.name", "value": { "stringValue": "{{ cpp_version }}" } },
          { "key": "process.runtime.version", "value": { "stringValue": "{{ tls_version }}" } }
        ]
      },
      "scopeMetrics": [
        {
          "metrics": [
            {
              "name": "{{ metric_name }}",
              "gauge": {
                "dataPoints": [
                  {
                    "asDouble": {{ metric_value }},
                    "attributes": [
                      {% for attr in attributes %}
                      {
                        "key": "{{ attr.key }}",
                        "value": { "stringValue": "{{ attr.val }}" }
                      }{% if not loop.is_last %},{% endif %}
                      {% endfor %}
                    ]
                  }
                ]
              }
            }
          ]
        }
      ]
    }
  ]
})";

class OtlpTemplateEngine {
public:
    static std::string render_payload(const core::Config& config, std::string_view name, double value, std::span<const sys::Attribute> attrs) {
        inja::Environment env;
        nlohmann::json data;

        data["program_name"] = config.app.program_name;
        data["hostname"]     = config.app.hostname;
        data["cpp_version"]  = config.app.cpp_version;
        // tls.version is "" for Plain builds (default) — render as "none".
        // Secure builds carry the negotiated version string set by the caller.
        data["tls_version"]  = config.tls.version.empty() ? "none" : config.tls.version;

        data["metric_name"]  = name;
        data["metric_value"] = value;

        nlohmann::json attr_array = nlohmann::json::array();
        for (const auto& attr : attrs) {
            attr_array.push_back({{"key", attr.key}, {"val", attr.val}});
        }
        data["attributes"] = attr_array;

        return env.render(otlp_json_template, data);
    }
};

} // namespace telemetry
#endif // OTLP_TEMPLATE_HPP
