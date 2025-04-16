#pragma once

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include <iostream>  // logging
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_set>

namespace lirs::tools
{
// constants
inline constexpr int ERROR_CODE = -1;
inline constexpr int CLOSED_HANDLE = -1;

inline constexpr timeval DEFAULT_SELECT_TIME = {1, 0};

inline constexpr size_t V4L2_MAX_BUFFER_SIZE = 32;
inline constexpr uint32_t DEFAULT_V4L2_BUFFERS_NUM = 4;

inline constexpr uint32_t DEFAULT_FRAME_RATE = 30;

inline constexpr uint32_t DEFAULT_FRAME_WIDTH = 640;
inline constexpr uint32_t DEFAULT_FRAME_HEIGHT = 480;

inline constexpr uint32_t DEFAULT_V4L2_PIXEL_FORMAT = V4L2_PIX_FMT_YUYV;

// Check if device is ready for reading
bool is_readable(int handle, timeval timeout = DEFAULT_SELECT_TIME)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(handle, &fds);

  const int ready = select(handle + 1, &fds, nullptr, nullptr, &timeout);

  if (ready == ERROR_CODE) {
    std::cerr << "ERROR: select() failed - " << strerror(errno) << '\n';
    return false;
  }

  return ready > 0;
}

// Safe ioctl wrapper with EINTR handling
template <typename T>
int xioctl(int handle, unsigned long request, T arg)
{
  int status{0};

  do {
    status = ioctl(handle, request, arg);
  } while (status == ERROR_CODE && errno == EINTR);

  return status;
}

// Check if device is a character device
bool is_character_device(const std::string& device)
{
  struct stat status;

  if (stat(device.c_str(), &status) == ERROR_CODE) {
    std::cerr << "ERROR: Cannot identify device " << device << " - " << strerror(errno) << '\n';
    return false;
  }

  if (!S_ISCHR(status.st_mode)) {
    std::cerr << "ERROR: " << device << " is not a character device\n";
    return false;
  }

  return true;
}

// Open V4L2 device
int open_device(const std::string& device)
{
  if (!is_character_device(device)) {
    return CLOSED_HANDLE;
  }

  const int handle = open(device.c_str(), O_RDWR | O_NONBLOCK);

  if (handle == CLOSED_HANDLE) {
    std::cerr << "ERROR: Cannot open device " << device << " - " << strerror(errno) << '\n';
  }

  return handle;
}

// Close V4L2 device
bool close_device(int handle)
{
  if (handle == CLOSED_HANDLE) {
    return false;
  }

  if (close(handle) == ERROR_CODE) {
    std::cerr << "ERROR: Cannot close device - " << strerror(errno) << '\n';
    return false;
  }

  return true;
}

// Query supported pixel formats
std::unordered_set<uint32_t> query_pixel_formats(int fd)
{
  std::unordered_set<uint32_t> formats;

  v4l2_fmtdesc desc = {};
  desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  while (xioctl(fd, VIDIOC_ENUM_FMT, &desc) != ERROR_CODE) {
    formats.insert(desc.pixelformat);
    desc.index++;
  }

  return formats;
}

// Check input capabilities
bool check_input_capabilities(int handle)
{
  v4l2_input input = {};

  if (xioctl(handle, VIDIOC_G_INPUT, &input.index) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_G_INPUT failed - " << strerror(errno) << '\n';
    return false;
  }

  if (xioctl(handle, VIDIOC_ENUMINPUT, &input) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_ENUMINPUT failed - " << strerror(errno) << '\n';
    return false;
  }

  if (input.type != V4L2_INPUT_TYPE_CAMERA) {
    std::cerr << "ERROR: Not a camera device\n";
    return false;
  }

  if (input.status & (V4L2_IN_ST_NO_POWER | V4L2_IN_ST_NO_SIGNAL)) {
    std::cerr << "ERROR: Device has power or signal issues\n";
    return false;
  }

  return true;
}

// Query device capabilities
std::optional<v4l2_capability> query_capabilities(int handle)
{
  v4l2_capability caps = {};

  if (xioctl(handle, VIDIOC_QUERYCAP, &caps) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_QUERYCAP failed - " << strerror(errno) << '\n';
    return std::nullopt;
  }

  return caps;
}

// Validate required capabilities
bool validate_capabilities(const v4l2_capability& caps, uint32_t required_caps)
{
  if ((caps.capabilities & required_caps) != required_caps) {
    std::cerr << "ERROR: Missing required capabilities\n";
    return false;
  }

  if (caps.capabilities & V4L2_CAP_TIMEPERFRAME) {
    std::cout << "NOTICE: Device supports frame rate control\n";
  }

  return true;
}

// Set video format
std::optional<v4l2_format> set_format(
  int handle, uint32_t pixel_format, uint32_t width, uint32_t height, bool try_format = false)
{
  v4l2_format fmt = {};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.pixelformat = pixel_format;
  fmt.fmt.pix.width = width;
  fmt.fmt.pix.height = height;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;

  const uint64_t req = try_format ? VIDIOC_TRY_FMT : VIDIOC_S_FMT;

  if (xioctl(handle, req, &fmt) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_[S/TRY]_FMT failed - " << strerror(errno) << '\n';
    return std::nullopt;
  }

  if (fmt.fmt.pix.pixelformat != pixel_format || fmt.fmt.pix.width != width || fmt.fmt.pix.height != height) {
    return std::nullopt;
  }

  return fmt;
}

std::optional<v4l2_format> set_format(int handle, v4l2_format format, bool try_format = false)
{
  const uint64_t req = try_format ? VIDIOC_TRY_FMT : VIDIOC_S_FMT;

  if (xioctl(handle, req, &format) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_[S/TRY]_FMT failed - " << strerror(errno) << '\n';
    return std::nullopt;
  }

  return format;
}

// Get current video format
std::optional<v4l2_format> get_format(int handle)
{
  v4l2_format fmt = {};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(handle, VIDIOC_G_FMT, &fmt) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_G_FMT failed - " << strerror(errno) << '\n';
    return std::nullopt;
  }

  return fmt;
}

std::optional<v4l2_streamparm> get_stream_params(int handle)
{
  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(handle, VIDIOC_G_PARM, &parm) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_G_PARM failed - " << strerror(errno) << '\n';
    return std::nullopt;
  }

  return parm;
}

// Set frame rate
std::optional<v4l2_streamparm> set_frame_rate(int handle, uint32_t numerator, uint32_t denominator)
{
  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator = numerator;
  parm.parm.capture.timeperframe.denominator = denominator;

  if (xioctl(handle, VIDIOC_S_PARM, &parm) == ERROR_CODE) {
    std::cerr << "ERROR: VIDIOC_S_PARM failed - " << strerror(errno) << '\n';
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
