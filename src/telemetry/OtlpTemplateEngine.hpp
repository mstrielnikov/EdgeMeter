#ifndef OTLP_TEMPLATE_HPP
#define OTLP_TEMPLATE_HPP

#include <string>
#include <map>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include "../core/Config.hpp"

namespace telemetry {

// Purely statically binds the rendering paths isolating templating contexts completely!
class OtlpTemplateEngine {
    // Encapsulate the raw exact OTLP JSON structured template formatting safely internally:
    static inline const std::string otlp_json_template = R"({
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

public:
    static std::string render_payload(const core::Config& config, const std::string& name, double value, const std::map<std::string, std::string>& attrs) {
        inja::Environment env;
        nlohmann::json data;
        
        data["program_name"] = config.program_name;
        data["hostname"] = config.hostname;
        data["cpp_version"] = config.cpp_version;
        data["tls_version"] = config.tls_version;
        
        data["metric_name"] = name;
        data["metric_value"] = value;
        
        nlohmann::json attr_array = nlohmann::json::array();
        for (const auto& kv : attrs) {
            attr_array.push_back({{"key", kv.first}, {"val", kv.second}});
        }
        data["attributes"] = attr_array;

        return env.render(otlp_json_template, data);
    }
};

} // namespace telemetry
#endif // OTLP_TEMPLATE_HPP
