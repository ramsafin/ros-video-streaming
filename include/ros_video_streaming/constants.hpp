#ifndef LIRS_CONSTANTS_HPP
#define LIRS_CONSTANTS_HPP

#include "ros_video_streaming/types.hpp"

namespace lirs::constants
{
inline constexpr auto V4L2_MAX_BUFFER_SIZE = types::BufferSize{32};
inline constexpr auto DEFAULT_V4L2_BUFFERS_NUM = uint32_t{4};
}  // namespace lirs::constants

#endif  // LIRS_CONSTANTS_HPP
