#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::telemetry {

enum class TelemetrySeverity {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3,
};

[[nodiscard]] std::string toString(TelemetrySeverity severity);
[[nodiscard]] TelemetrySeverity severityFromCompatTag(const std::string& tag);

struct TelemetryEvent {
    std::string subsystem;
    std::string name;
    TelemetrySeverity severity = TelemetrySeverity::Info;
    std::string code;
    std::string message;
    std::map<std::string, std::string> fields;

    [[nodiscard]] nlohmann::json toJson() const;
};

class TelemetrySink {
  public:
    void setEnabled(bool enabled);
    [[nodiscard]] bool enabled() const;
    bool capture(const TelemetryEvent& event);
    [[nodiscard]] const std::vector<TelemetryEvent>& events() const;

  private:
    bool enabled_ = true;
    std::vector<TelemetryEvent> events_;
};

} // namespace urpg::telemetry
