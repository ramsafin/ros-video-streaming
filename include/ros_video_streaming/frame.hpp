#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

namespace lirs {

class Frame final {
public:
  Frame() = default;

  Frame(const uint8_t* data, size_t length, uint64_t timestamp, uint64_t sequence = 0)
  : data_{data, data + length}, sequence_(sequence), timestamp_{timestamp} {
    // ...
  }

  // Copy
  Frame(const Frame&) = default;
  Frame& operator=(const Frame&) = default;

  // Move
  Frame(Frame&&) noexcept = default;
  Frame& operator=(Frame&&) noexcept = default;

  // Accessors
  const std::vector<uint8_t>& data() const {
    return data_;
  }

  uint64_t timestamp() const {
    return timestamp_;
  }

  uint64_t sequence() const {
    return sequence_;
  }

private:
  uint64_t sequence_;
  uint64_t timestamp_;
  std::vector<uint8_t> data_;
};

}  // namespace lirs
