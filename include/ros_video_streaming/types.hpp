#ifndef LIRS_TYPES_HPP
#define LIRS_TYPES_HPP

#include <linux/videodev2.h>

#include <cstddef>  // size_t
#include <cstdint>  // uint32_t
#include <string_view>

namespace lirs::types {
using PixelFormat = decltype(V4L2_PIX_FMT_RGB24);

using FileDescriptor = int;

using BufferSize = size_t;

using FrameWidth = uint32_t;
using FrameHeight = uint32_t;

struct InputType {
  uint32_t value;

  constexpr explicit InputType(uint32_t v) : value{v} {}
  constexpr operator uint32_t() const { return value; }

  constexpr std::string_view name() const {
    switch (value) {
      case V4L2_INPUT_TYPE_TUNER:
        return "tuner";
      case V4L2_INPUT_TYPE_CAMERA:
        return "camera";
      case V4L2_INPUT_TYPE_TOUCH:
        return "touch";
      default:
        return "unknown";
    }
  }
};

struct InputStatus {
  uint32_t value;

  constexpr explicit InputStatus(uint32_t v) : value{v} {}
  constexpr operator uint32_t() const { return value; }

  constexpr std::string_view name() const {
    switch (value) {
      case V4L2_IN_ST_NO_POWER:
        return "no power";
      case V4L2_IN_ST_NO_SIGNAL:
        return "no signal";
      case V4L2_IN_ST_NO_COLOR:
        return "no color";
      default:
        return "unknown";
    }
  }
};

}  // namespace lirs::types

#endif  // LIRS_TYPES_HPP
