#ifndef LIRS_TOOLS_HPP
#define LIRS_TOOLS_HPP

#include <plog/Log.h>

#include <linux/videodev2.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include <algorithm>  // sort
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "ros_video_streaming/formats.hpp"
#include "ros_video_streaming/types.hpp"

namespace lirs::tools {

namespace details {
inline constexpr auto IOCTL_ERROR_CODE = int{-1};
inline constexpr auto DEFAULT_SELECT_TIME = timeval{1, 0};  // (secs, microsecs)
}  // namespace details

// Check if device is ready for reading
inline bool is_readable(types::descriptor_t fd, timeval timeout = details::DEFAULT_SELECT_TIME) {
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
int xioctl(types::descriptor_t fd, unsigned long request, T arg) {
  int ret;

  do {
    ret = ioctl(fd, request, arg);
  } while (ret == details::IOCTL_ERROR_CODE && errno == EINTR);

  if (errno == EINVAL) {
    PLOG_VERBOSE.printf("ioctl() failed: fd = %d. %s", strerror(errno));
    return ret;
  }

  if (ret == details::IOCTL_ERROR_CODE) {
    PLOG_WARNING.printf("ioctl() failed: fd = %d. %s", fd, strerror(errno));
  }

  return ret;
}

inline bool is_character_device(const std::string& device) {
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

inline types::descriptor_t open_device(const std::string& device) {
  const types::descriptor_t fd = open(device.c_str(), O_RDWR | O_NONBLOCK);

  if (fd == -1) {
    PLOG_WARNING.printf("Cannot open device: %s. %s", device.c_str(), strerror(errno));
  }

  PLOG_INFO.printf("Opened device: %s, fd = %d", device.c_str(), fd);

  return fd;
}

inline bool close_device(types::descriptor_t fd) {
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

inline std::vector<v4l2_input> list_available_inputs(types::descriptor_t fd) {
  auto available_inputs = std::vector<v4l2_input>{};
  auto input = v4l2_input{};

  for (input.index = 0; xioctl(fd, VIDIOC_ENUMINPUT, &input) == 0; input.index++) {
    available_inputs.push_back(input);
  }

  return available_inputs;
}

inline std::optional<v4l2_capability> query_capabilities(types::descriptor_t fd) {
  auto caps = v4l2_capability{};

  if (xioctl(fd, VIDIOC_QUERYCAP, &caps) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_QUERYCAP failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  return caps;
}

inline bool check_video_streaming_caps(uint32_t caps) {
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

inline std::vector<v4l2_fmtdesc> list_pixel_formats(types::descriptor_t fd) {
  auto pix_formats = std::vector<v4l2_fmtdesc>{};
  pix_formats.reserve(8);

  auto format = v4l2_fmtdesc{};
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  for (format.index = 0; xioctl(fd, VIDIOC_ENUM_FMT, &format) == 0; format.index++) {
    pix_formats.push_back(format);
  }

  if (pix_formats.empty()) {
    PLOG_WARNING.printf("No supported pixel formats: fd = %d", fd);
  }

  return pix_formats;
}

inline std::vector<v4l2_frmsizeenum> list_frame_sizes(types::descriptor_t fd, types::pix_format_t pix_format) {
  auto frame_sizes = std::vector<v4l2_frmsizeenum>{};
  frame_sizes.reserve(16);

  auto size = v4l2_frmsizeenum{};
  size.pixel_format = pix_format;

  for (size.index = 0; xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &size) == 0; size.index++) {
    if (size.type != V4L2_FRMSIZE_TYPE_DISCRETE) {
      PLOG_WARNING << "Continuous or stepwise frame sizes are not handled";
      continue;
    }

    frame_sizes.push_back(size);
  }

  if (frame_sizes.empty()) {
    PLOG_WARNING.printf("No supported frame sizes: fd = %d, format = %s", fd, formats::format2str(pix_format).data());
  }

  std::sort(std::begin(frame_sizes), std::end(frame_sizes), [](const auto& left, const auto& right) {
    return left.discrete.width < right.discrete.width;
  });

  return frame_sizes;
}

inline std::vector<v4l2_frmivalenum> list_frame_rates(
  types::descriptor_t fd, types::pix_format_t pix_format, types::width_t width, types::height_t height) {
  auto frame_rates = std::vector<v4l2_frmivalenum>{};
  frame_rates.reserve(8);

  auto fps = v4l2_frmivalenum{};
  fps.pixel_format = pix_format;
  fps.width = width;
  fps.height = height;

  for (fps.index = 0; xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fps) == 0; fps.index++) {
    if (fps.type != V4L2_FRMIVAL_TYPE_DISCRETE) {
      PLOG_WARNING << "Continuous or stepwise frame rates are not handled";
      continue;
    }

    frame_rates.push_back(fps);
  }

  if (frame_rates.empty()) {
    PLOG_WARNING.printf(
      "No supported frame rates: fd = %d, format = %s @ %d x %d", fd, formats::format2str(pix_format).data(), width,
      height);
  }

  std::sort(std::begin(frame_rates), std::end(frame_rates), [](const auto& left, const auto& right) {
    return left.discrete.numerator < right.discrete.numerator;
  });

  return frame_rates;
}

// Get current video format
inline std::optional<v4l2_format> get_format(types::descriptor_t fd) {
  v4l2_format fmt = {};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(fd, VIDIOC_G_FMT, &fmt) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_G_FMT failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  return fmt;
}

inline std::optional<v4l2_streamparm> get_stream_params(types::descriptor_t fd) {
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
  types::descriptor_t fd, types::pix_format_t format, types::width_t width, types::height_t height,
  bool try_format = false) {
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
inline std::optional<v4l2_streamparm> set_frame_rate(types::descriptor_t fd, uint32_t num, uint32_t den) {
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
constexpr bool is_in_range(T low, T high, T value) noexcept {
  return value >= low && value <= high;
}

}  // namespace lirs::tools

#endif  // LIRS_TOOLS_HPP
