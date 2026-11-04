#pragma once

#include <cstddef>
#include <cstdint>

namespace esphome::openquatt_common {

enum class ExternalTelemetryPublishResult : unsigned char {
  NONE = 0U,
  SUCCEEDED = 1U,
  FAILED = 2U,
  CANCELLED = 3U,
};

// Narrow outbound-only interface shared by telemetry producers. It keeps the
// MQTT client and its worker stack in one owner while consent and retry policy
// remain owned by the feature that collected the data.
class OpenQuattExternalTelemetryTransport {
 public:
  virtual ~OpenQuattExternalTelemetryTransport() = default;
  virtual bool ensure_installation_id_for_external() = 0;
  virtual const char* external_installation_id() const = 0;
  virtual int64_t external_publish_next_allowed_us() const = 0;
  virtual bool request_external_publish(const char* suffix, const char* payload, size_t payload_size) = 0;
  virtual void cancel_external_publish() = 0;
  virtual ExternalTelemetryPublishResult take_external_publish_result() = 0;
};

}  // namespace esphome::openquatt_common
