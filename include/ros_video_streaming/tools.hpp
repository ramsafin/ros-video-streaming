#ifndef LIRS_TOOLS_HPP
#define LIRS_TOOLS_HPP

#include "ros_video_streaming/inputs.hpp"
#include "ros_video_streaming/types.hpp"

#include <linux/videodev2.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <plog/Log.h>

namespace lirs::tools
{
namespace details
{
inline constexpr auto IOCTL_ERROR_CODE = int{-1};
inline constexpr auto DEFAULT_SELECT_TIME = timeval{1, 0};  // (secs, microsecs)
inline constexpr auto CLOSED_HANDLE = types::FileDescriptor{-1};

}  // namespace details

// Check if device is ready for reading
inline bool is_readable(types::FileDescriptor fd, timeval timeout = details::DEFAULT_SELECT_TIME)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  const int ready = select(fd + 1, &fds, nullptr, nullptr, &timeout);

  if (ready == details::IOCTL_ERROR_CODE) {
    PLOG_WARNING.printf("select() failed: fd = %d. %s", fd, strerror(errno));
    return false;
  }

  return ready > 0;
}

template <typename T>
int xioctl(types::FileDescriptor fd, unsigned long request, T arg)
{
  int ret;

  do {
    ret = ioctl(fd, request, arg);
  } while (ret == details::IOCTL_ERROR_CODE && errno == EINTR);

  if (ret == details::IOCTL_ERROR_CODE) {
    PLOG_WARNING.printf("ioctl() failed: fd = %d. %s", fd, strerror(errno));
  }

  return ret;
}

inline bool is_character_device(const std::string& device)
{
  struct stat status;

  if (stat(device.c_str(), &status) == details::IOCTL_ERROR_CODE) {
    PLOG_WARNING.printf("Cannot identify device: %s. %s", device.c_str(), strerror(errno));
    return false;
  }

  if (!S_ISCHR(status.st_mode)) {
    PLOG_WARNING.printf("Not a character device: %s. %s", device.c_str(), strerror(errno));
    return false;
  }

  return true;
}

inline types::FileDescriptor open_device(const std::string& device)
{
  const types::FileDescriptor fd = open(device.c_str(), O_RDWR | O_NONBLOCK);

  if (fd == details::CLOSED_HANDLE) {
    PLOG_WARNING.printf("Cannot open device: %s. %s", device.c_str(), strerror(errno));
  }

  PLOG_INFO.printf("Opened device: %s, fd = %d", device.c_str(), fd);

  return fd;
}

inline bool close_device(types::FileDescriptor fd)
{
  if (fd < 0) {
    PLOG_WARNING.printf("Invalid file descriptor: %d", fd);
    return false;
  }

  if (close(fd) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("Cannot close device: fd = %d. %s", fd, strerror(errno));
    return false;
  }

  return true;
}

inline std::vector<v4l2_input> list_available_inputs(types::FileDescriptor fd)
{
  auto inputs = std::vector<v4l2_input>{};
  auto input = v4l2_input{};

  for (input.index = 0; xioctl(fd, VIDIOC_ENUMINPUT, &input) == 0; input.index++) {
    inputs.push_back(input);
  }

  return inputs;
}

inline bool check_video_input(types::FileDescriptor fd, uint32_t index)
{
  auto input = v4l2_input{};
  input.index = index;

  if (xioctl(fd, VIDIOC_G_INPUT, &input.index) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_G_INPUT failed: fd = %d, index = %d. %s", fd, index, strerror(errno));
    return false;
  }

  if (input.type != V4L2_INPUT_TYPE_CAMERA) {
    PLOG_WARNING.printf("Not a video input: fd = %d, index = %d", fd, index);
    return false;
  }

  if (input.status & (V4L2_IN_ST_NO_POWER | V4L2_IN_ST_NO_SIGNAL)) {
    PLOG_WARNING.printf("Device has power/signal issues: fd = %d, index = %d", fd, index);
    return false;
  }

  return true;
}

inline std::optional<v4l2_capability> query_capabilities(types::FileDescriptor fd)
{
  auto caps = v4l2_capability{};

  if (xioctl(fd, VIDIOC_QUERYCAP, &caps) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_QUERYCAP failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  return caps;
}

inline bool check_video_streaming_caps(uint32_t caps)
{
  if (!(caps & V4L2_CAP_VIDEO_CAPTURE)) {
    PLOG_WARNING << "V4L2_CAP_VIDEO_CAPTURE not supported";
    return false;
  }

  if (!(caps & V4L2_CAP_STREAMING)) {
    PLOG_WARNING << "V4L2_CAP_STREAMING not supported";
    return false;
  }

  return true;
}

inline std::vector<v4l2_fmtdesc> list_pixel_formats(types::FileDescriptor fd)
{
  auto formats = std::vector<v4l2_fmtdesc>{};
  formats.reserve(3);

  auto format = v4l2_fmtdesc{};
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  while (xioctl(fd, VIDIOC_ENUM_FMT, &format) != details::IOCTL_ERROR_CODE) {
    formats.push_back(format);
    format.index++;
  }

  if (formats.empty()) {
    PLOG_WARNING.printf("No supported pixel formats: fd = %d", fd);
  }

  return formats;
}

// Get current video format
inline std::optional<v4l2_format> get_format(types::FileDescriptor fd)
{
  v4l2_format fmt = {};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(fd, VIDIOC_G_FMT, &fmt) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_G_FMT failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  return fmt;
}

inline std::optional<v4l2_streamparm> get_stream_params(types::FileDescriptor fd)
{
  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(fd, VIDIOC_G_PARM, &parm) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_G_PARM failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  return parm;
}

// Set video format
inline std::optional<v4l2_format> set_format(
  types::FileDescriptor fd, types::PixelFormat format, types::FrameWidth width,
  types::FrameHeight height, bool try_format = false)
{
  v4l2_format fmt = {};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.pixelformat = format;
  fmt.fmt.pix.width = width;
  fmt.fmt.pix.height = height;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;

  const uint64_t req = try_format ? VIDIOC_TRY_FMT : VIDIOC_S_FMT;

  if (xioctl(fd, req, &fmt) == details::IOCTL_ERROR_CODE) {
    const auto req = try_format ? "VIDIOC_TRY_FMT" : "VIDIOC_S_FMT";
    PLOG_ERROR.printf("%s failed: fd = %d. %s", req, fd, strerror(errno));
    return std::nullopt;
  }

  return fmt;
}

// Set frame rate
inline std::optional<v4l2_streamparm> set_frame_rate(
  types::FileDescriptor fd, uint32_t num, uint32_t den)
{
  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator = num;
  parm.parm.capture.timeperframe.denominator = den;

  if (xioctl(fd, VIDIOC_S_PARM, &parm) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_S_PARM failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  return parm;
}

// Range check helper
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr bool is_in_range(T low, T high, T value) noexcept
{
  return value >= low && value <= high;
}

}  // namespace lirs::tools

#endif  // LIRS_TOOLS_HPP
