#ifndef LIRS_TOOLS_HPP
#define LIRS_TOOLS_HPP

#include "ros_video_streaming/types.hpp"

#include <linux/videodev2.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include <optional>
#include <string>
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
    PLOG_ERROR << "select() failed. " << strerror(errno);
    return false;
  }

  return ready > 0;
}

// Safe ioctl wrapper with EINTR handling
template <typename T>
int xioctl(types::FileDescriptor fd, unsigned long request, T arg)
{
  int ret;

  do {
    ret = ioctl(fd, request, arg);
  } while (ret == details::IOCTL_ERROR_CODE && errno == EINTR);

  if (ret == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR << "ioctl() failed. " << strerror(errno);
  }

  return ret;
}

// Check if device is a character device
inline bool is_character_device(const std::string& device)
{
  struct stat status;

  if (stat(device.c_str(), &status) == details::IOCTL_ERROR_CODE) {
    PLOG_WARNING.printf("Cannot identify device: %s. %s", device, strerror(errno));
    return false;
  }

  if (!S_ISCHR(status.st_mode)) {
    PLOG_WARNING.printf("Not a character device: %s. %s", device, strerror(errno));
    return false;
  }

  return true;
}

// Open V4L2 device
inline types::FileDescriptor open_device(const std::string& device)
{
  if (!is_character_device(device)) {
    return details::CLOSED_HANDLE;
  }

  const int fd = open(device.c_str(), O_RDWR | O_NONBLOCK);

  if (fd == details::CLOSED_HANDLE) {
    PLOG_WARNING.printf("Cannot open device: %s. %s", device, strerror(errno));
  }

  return fd;
}

// Close V4L2 device
inline bool close_device(types::FileDescriptor fd)
{
  if (fd == details::CLOSED_HANDLE) {
    PLOG_WARNING.printf("Invalid file descriptor: %d", fd);
    return false;
  }

  if (close(fd) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("Cannot close device: fd = %d. %s", fd, strerror(errno));
    return false;
  }

  return true;
}

// Check input capabilities
inline bool check_input_capabilities(types::FileDescriptor fd)
{
  v4l2_input input = {};

  if (xioctl(fd, VIDIOC_G_INPUT, &input.index) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_G_INPUT failed: fd = %d. %s", fd, strerror(errno));
    return false;
  }

  if (xioctl(fd, VIDIOC_ENUMINPUT, &input) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_ENUMINPUT failed: fd = %d. %s", fd, strerror(errno));
    return false;
  }

  if (input.type != V4L2_INPUT_TYPE_CAMERA) {
    PLOG_ERROR.printf("Incorrect input type: fd = %d. Expected V4L2_INPUT_TYPE_CAMERA", fd);
    return false;
  }

  if (input.status & (V4L2_IN_ST_NO_POWER | V4L2_IN_ST_NO_SIGNAL)) {
    PLOG_ERROR.printf("Device has power/signal issues: fd = %d", fd);
    return false;
  }

  return true;
}

// Query supported pixel formats
inline std::vector<uint32_t> query_pixel_formats(types::FileDescriptor fd)
{
  std::vector<uint32_t> formats;
  formats.reserve(10);

  v4l2_fmtdesc desc = {};
  desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  while (xioctl(fd, VIDIOC_ENUM_FMT, &desc) != details::IOCTL_ERROR_CODE) {
    formats.push_back(desc.pixelformat);
    desc.index++;
  }

  if (formats.empty()) {
    PLOG_WARNING.printf("Empty list of supported pixel formats: fd = %d", fd);
  }

  return formats;
}

// Query device capabilities
inline std::optional<v4l2_capability> query_capabilities(types::FileDescriptor fd)
{
  v4l2_capability caps = {};

  if (xioctl(fd, VIDIOC_QUERYCAP, &caps) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_QUERYCAP failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  return caps;
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
