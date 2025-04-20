#ifndef LIRS_FORMATS_HPP
#define LIRS_FORMATS_HPP

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#include "ros_video_streaming/types.hpp"

#include <linux/videodev2.h>  // V4L2_PIX_FMT_* are listed here

#include <array>
#include <string_view>

namespace lirs::formats {
namespace details {
struct PixelFormatInfo {
  types::PixelFormat format;
  std::string_view name;
};

inline constexpr auto PIXEL_FORMAT_TABLE = std::array<PixelFormatInfo, 7>{{
  {V4L2_PIX_FMT_MJPEG, "MJPEG"},
  {V4L2_PIX_FMT_H264, "H264"},
  {V4L2_PIX_FMT_HEVC, "HEVC"},
  {V4L2_PIX_FMT_YUYV, "YUYV"},
  {V4L2_PIX_FMT_RGB24, "RGB24"},
  {V4L2_PIX_FMT_BGR24, "BGR24"},
  {V4L2_PIX_FMT_GREY, "GREY"},
  // ... add formats as needed
}};
}  // namespace details

inline constexpr auto V4L2_PIX_FMT_UNKNOWN = types::PixelFormat{0};
inline constexpr auto V4L2_PIX_NAME_UNKNOWN = std::string_view{"Unknown"};

inline constexpr std::string_view format2str(types::PixelFormat format) {
  for (const auto& entry : details::PIXEL_FORMAT_TABLE) {
    if (entry.format == format) {
      return entry.name;
    }
  }
  return V4L2_PIX_NAME_UNKNOWN;
}

inline constexpr types::PixelFormat str2format(std::string_view name) {
  for (const auto& entry : details::PIXEL_FORMAT_TABLE) {
    if (entry.name == name) {
      return entry.format;
    }
  }
  return V4L2_PIX_FMT_UNKNOWN;
}

}  // namespace lirs::formats

#endif  // LIRS_FORMATS_HPP