#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace esphome::openquatt_performance_telemetry {

inline constexpr uint8_t PERFORMANCE_SAMPLES_PER_MINUTE = 6U;
inline constexpr float PERFORMANCE_MIN_INPUT_POWER_W = 100.0f;
inline constexpr float PERFORMANCE_MIN_THERMAL_POWER_W = 100.0f;
inline constexpr int64_t PERFORMANCE_PUBLISH_INTERVAL_US = 15LL * 60LL * 1000000LL;

inline bool performance_publish_due(int64_t now_us, int64_t deadline_us) {
  return deadline_us > 0 && now_us >= deadline_us;
}

inline int64_t advance_performance_publish_deadline(int64_t deadline_us, int64_t now_us) {
  if (deadline_us <= 0) return now_us + PERFORMANCE_PUBLISH_INTERVAL_US;
  do {
    deadline_us += PERFORMANCE_PUBLISH_INTERVAL_US;
  } while (deadline_us <= now_us);
  return deadline_us;
}

class FixedBufferWriter {
 public:
  FixedBufferWriter(char* data, size_t capacity) : data_(data), capacity_(capacity) {
    if (this->data_ == nullptr || this->capacity_ == 0U) {
      this->ok_ = false;
    } else {
      this->data_[0] = '\0';
    }
  }

  FixedBufferWriter& operator+=(const char* value) {
    if (value != nullptr) this->append_(value, std::strlen(value));
    return *this;
  }
  FixedBufferWriter& operator+=(const std::string& value) {
    this->append_(value.data(), value.size());
    return *this;
  }
  FixedBufferWriter& operator+=(char value) {
    this->append_(&value, 1U);
    return *this;
  }
  void append_uint(uint64_t value) {
    char buffer[24];
    const int length = std::snprintf(buffer, sizeof(buffer), "%llu", static_cast<unsigned long long>(value));
    if (length <= 0) {
      this->ok_ = false;
      return;
    }
    this->append_(buffer, static_cast<size_t>(length));
  }
  bool ok() const { return this->ok_; }
  size_t size() const { return this->size_; }

 private:
  void append_(const char* value, size_t length) {
    if (!this->ok_ || value == nullptr || length >= this->capacity_ - this->size_) {
      this->ok_ = false;
      return;
    }
    std::memcpy(this->data_ + this->size_, value, length);
    this->size_ += length;
    this->data_[this->size_] = '\0';
  }

  char* data_{nullptr};
  size_t capacity_{0U};
  size_t size_{0U};
  bool ok_{true};
};

inline void append_json_escaped(FixedBufferWriter& output, const std::string& input) {
  for (char c : input) {
    switch (c) {
      case '"':
        output += "\\\"";
        break;
      case '\\':
        output += "\\\\";
        break;
      case '\b':
        output += "\\b";
        break;
      case '\f':
        output += "\\f";
        break;
      case '\n':
        output += "\\n";
        break;
      case '\r':
        output += "\\r";
        break;
      case '\t':
        output += "\\t";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20U) {
          char buffer[7];
          std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned char>(c));
          output += buffer;
        } else {
          output += c;
        }
        break;
    }
  }
}

inline void append_json_string(FixedBufferWriter& output, const std::string& input) {
  output += '"';
  append_json_escaped(output, input);
  output += '"';
}

inline bool stable_minute(uint8_t samples, bool invalid, uint8_t active_mask) {
  return samples == PERFORMANCE_SAMPLES_PER_MINUTE && !invalid && active_mask != 0U;
}

// A zero deadline is the explicit "publish immediately" sentinel. Check it
// before wrap-safe signed subtraction, which is ambiguous after 2^31 ms.
inline bool retry_due(uint32_t now_ms, uint32_t target_ms) {
  return target_ms == 0U || static_cast<int32_t>(now_ms - target_ms) >= 0;
}

// Reject the legacy estimator's near-zero fallback while a compressor reports
// active. The low floors preserve ordinary low-load heating samples.
inline bool valid_active_measurement(float input_power_w, float thermal_power_w, float pump_power_w) {
  return std::isfinite(input_power_w) && std::isfinite(thermal_power_w) && std::isfinite(pump_power_w) &&
         input_power_w >= PERFORMANCE_MIN_INPUT_POWER_W && thermal_power_w >= PERFORMANCE_MIN_THERMAL_POWER_W &&
         pump_power_w >= 0.0f;
}

}  // namespace esphome::openquatt_performance_telemetry
