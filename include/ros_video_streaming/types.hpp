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

using caps_t = decltype(v4l2_capability::capabilities);
using pix_format_t = decltype(v4l2_fmtdesc::pixelformat);
using fps_t = decltype(v4l2_frmivalenum::discrete.numerator);

struct Resolution {
  width_t width;
  height_t height;

  constexpr width_t total() const { return width * height; }

  bool operator==(const Resolution& other) const { return width == other.width && height == other.height; }
};

namespace details {

struct ResolutionHash {
  std::size_t operator()(const Resolution& resolution) const {
    std::size_t h1 = std::hash<width_t>{}(resolution.width);
    std::size_t h2 = std::hash<height_t>{}(resolution.height);
    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

}  // namespace details

struct FrameRate {
  fps_t num;
  fps_t den;

  constexpr double as_double() const { return den == 0 ? 0.0 : static_cast<double>(den) / num; }
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

using FrameRateList = std::vector<FrameRate>;
using ResolutionList = std::vector<Resolution>;
using PixFormatList = std::vector<pix_format_t>;

using ResolutionMap = std::unordered_map<Resolution, FrameRateList, details::ResolutionHash>;

using FormatMap = std::unordered_map<pix_format_t, ResolutionMap>;

}  // namespace lirs::types

#endif  // LIRS_TYPES_HPP
