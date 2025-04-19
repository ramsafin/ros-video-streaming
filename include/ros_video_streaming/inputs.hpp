#ifndef LIRS_INPUTS_HPP
#define LIRS_INPUTS_HPP

#include "ros_video_streaming/types.hpp"

#include <linux/videodev2.h>  // V4L2_INPUT_* are listed here

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace lirs::inputs
{
namespace details
{
struct InputTypeInfo
{
  uint32_t type;
  std::string_view name;
};

struct InputStatusInfo
{
  uint32_t status;
  std::string_view name;
};

inline constexpr auto INPUT_TYPE_TABLE = std::array<InputTypeInfo, 3>{{

  {V4L2_INPUT_TYPE_TUNER, "tuner"},
  {V4L2_INPUT_TYPE_CAMERA, "camera"},
  {V4L2_INPUT_TYPE_TOUCH, "touch"},
  // ...
}};

inline constexpr auto INPUT_STATUS_TABLE = std::array<InputStatusInfo, 2>{{
  {V4L2_IN_ST_NO_POWER, "no power"},
  {V4L2_IN_ST_NO_SIGNAL, "no signal"},
}};
}  // namespace details

inline constexpr std::string_view type2str(uint32_t type)
{
  for (const auto& entry : details::INPUT_TYPE_TABLE) {
    if (entry.type == type) {
      return entry.name;
    }
  }
  return "unknown";
}

inline constexpr std::string_view status2str(uint32_t status)
{
  for (const auto& entry : details::INPUT_STATUS_TABLE) {
    if (entry.status == status) {
      return entry.name;
    }
  }
  return "unknown";
}

}  // namespace lirs::inputs

#endif  // LIRS_INPUTS_HPP
