#ifndef LIRS_TYPES_HPP
#define LIRS_TYPES_HPP

#include <linux/videodev2.h>
#include <cstddef>  // size_t
#include <cstdint>  // uint32_t

namespace lirs::types
{
using PixelFormat = decltype(V4L2_PIX_FMT_RGB24);

using FileDescriptor = int;

using BufferSize = size_t;

using FrameWidth = uint32_t;
using FrameHeight = uint32_t;

}  // namespace lirs::types

#endif  // LIRS_TYPES_HPP
