#ifndef LIRS_TYPES_HPP
#define LIRS_TYPES_HPP

#include <linux/videodev2.h>

#include <cstddef>  // size_t
#include <cstdint>  // uint32_t
#include <string_view>
#include <unordered_map>
#include <utility>  // pair

namespace lirs::types {
using descriptor_t = int;
using buffer_size_t = size_t;

using width_t = decltype(v4l2_frmsizeenum::discrete.width);
using height_t = decltype(v4l2_frmsizeenum::discrete.height);

using input_type_t = decltype(v4l2_input::type);
using input_stat_t = decltype(v4l2_input::status);

using pix_format_t = decltype(v4l2_fmtdesc::pixelformat);
using fps_t = decltype(v4l2_frmivalenum::discrete.numerator);

struct Resolution {
  width_t width;
  height_t height;
};

struct FrameRate {
  fps_t num;
  fps_t den;
};

struct InputType {
  input_type_t value;

  constexpr explicit InputType(input_type_t v) : value{v} {}
  constexpr operator input_type_t() const { return value; }

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
  input_stat_t value;

  constexpr explicit InputStatus(input_stat_t v) : value{v} {}
  constexpr operator input_stat_t() const { return value; }

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

namespace details {
struct pair_hash {
  template <class T1, class T2>
  size_t operator()(const std::pair<T1, T2>& p) const {
    const auto h1 = std::hash<T1>{}(p.first);
    const auto h2 = std::hash<T2>{}(p.second);
    return h1 ^ (h2 << 1);
  }
};
}  // namespace details

using FrameRateList = std::vector<FrameRate>;
using ResolutionMap = std::unordered_map<Resolution, FrameRateList, details::pair_hash>;

using CapabilityMap = std::unordered_map<pix_format_t, ResolutionMap>;

}  // namespace lirs::types

#endif  // LIRS_TYPES_HPP
