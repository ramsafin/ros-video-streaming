#ifndef LIRS_TOOLS_HPP
#define LIRS_TOOLS_HPP

#include <plog/Log.h>

#include <linux/videodev2.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
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

inline bool is_readable(types::descriptor_t fd, timeval timeout = details::DEFAULT_SELECT_TIME) {
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(fd, &fds);

  const int ready = select(fd + 1, &fds, nullptr, nullptr, &timeout);

  if (ready == -1) {
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

inline bool check_video_streaming_caps(types::caps_t caps) {
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

inline bool check_timeperframe_caps(types::caps_t caps) {
  if (caps & V4L2_CAP_TIMEPERFRAME) {
    return true;
  }

  return false;
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

inline std::vector<v4l2_frmsizeenum> list_frame_sizes(types::descriptor_t fd, formats::PixelFormat fmt) {
  auto frame_sizes = std::vector<v4l2_frmsizeenum>{};
  frame_sizes.reserve(16);

  auto size = v4l2_frmsizeenum{};
  size.pixel_format = formats::format2fourcc(fmt);

  for (size.index = 0; xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &size) == 0; size.index++) {
    if (size.type != V4L2_FRMSIZE_TYPE_DISCRETE) {
      PLOG_WARNING << "Continuous or stepwise frame sizes are not handled";
      continue;
    }

    frame_sizes.push_back(size);
  }

  if (frame_sizes.empty()) {
    PLOG_WARNING.printf("No supported frame sizes: fd = %d, format = %s", fd, formats::format2str(fmt).data());
  }

  const auto greater_or_eq = [](const v4l2_frmsizeenum& left, const v4l2_frmsizeenum& right) -> bool {
    return left.discrete.width >= right.discrete.width && left.discrete.height >= right.discrete.height;
  };

  std::sort(std::begin(frame_sizes), std::end(frame_sizes), greater_or_eq);

  return frame_sizes;
}

inline std::vector<v4l2_frmivalenum> list_frame_rates(
  types::descriptor_t fd, formats::PixelFormat fmt, const types::Resolution& res) {
  auto frame_rates = std::vector<v4l2_frmivalenum>{};
  frame_rates.reserve(8);

  auto fps = v4l2_frmivalenum{};

  fps.width = res.width;
  fps.height = res.height;
  fps.pixel_format = formats::format2fourcc(fmt);

  for (fps.index = 0; xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &fps) == 0; fps.index++) {
    if (fps.type != V4L2_FRMIVAL_TYPE_DISCRETE) {
      PLOG_WARNING << "Continuous or stepwise frame rates are not handled";
      continue;
    }

    frame_rates.push_back(fps);
  }

  if (frame_rates.empty()) {
    PLOG_WARNING.printf(
      "No supported frame rates: fd = %d, format = %s @ %d x %d", fd, formats::format2str(fmt).data(), res.width,
      res.height);
  }

  const auto greater_or_eq = [](const v4l2_frmivalenum& left, const v4l2_frmivalenum& right) {
    const auto left_fps = left.discrete.numerator / static_cast<float>(left.discrete.denominator);
    const auto right_fps = right.discrete.numerator / static_cast<float>(right.discrete.denominator);

    return left_fps >= right_fps;
  };

  std::sort(std::begin(frame_rates), std::end(frame_rates), greater_or_eq);

  return frame_rates;
}

inline std::optional<v4l2_format> set_format(
  types::descriptor_t fd, formats::PixelFormat fmt, const types::Resolution& res) {
  auto format = v4l2_format{};

  format.fmt.pix.pixelformat = formats::format2fourcc(fmt);
  format.fmt.pix.field = V4L2_FIELD_ANY;
  format.fmt.pix.width = res.width;
  format.fmt.pix.height = res.height;
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (xioctl(fd, VIDIOC_S_FMT, &format) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_S_FMT failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  // verify format
  if (format.fmt.pix.width != res.width || format.fmt.pix.height != res.height) {
    PLOG_WARNING.printf(
      "Resolution adjusted by device: requested %dx%d, got %dx%d", res.width, res.height, format.fmt.pix.width,
      format.fmt.pix.height);
  }

  const auto current_format = formats::fourcc2format(format.fmt.pix.pixelformat);

  if (format.fmt.pix.pixelformat != formats::format2fourcc(fmt)) {
    PLOG_WARNING.printf(
      "Pixel format adjusted by device: requested %s, got %s", formats::format2str(fmt).data(),
      formats::format2str(*current_format).data());
  }

  return format;
}

inline std::optional<v4l2_streamparm> set_frame_rate(types::descriptor_t fd, const types::FrameRate& fps) {
  auto parm = v4l2_streamparm{};

  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator = fps.num;
  parm.parm.capture.timeperframe.denominator = fps.den;

  if (xioctl(fd, VIDIOC_S_PARM, &parm) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("VIDIOC_S_PARM failed: fd = %d. %s", fd, strerror(errno));
    return std::nullopt;
  }

  const auto current_fps = parm.parm.capture.timeperframe;

  if (current_fps.numerator != fps.num || current_fps.denominator != fps.den) {
    PLOG_WARNING.printf(
      "Framerate adjusted by device: requested %d/%d, got %d/%d", fps.den, fps.num, current_fps.denominator,
      current_fps.numerator);
  }
  return parm;
}

inline std::optional<size_t> request_mmap_buffers(types::descriptor_t fd, size_t num_buffers = 4) {
  auto buf_request = v4l2_requestbuffers{};
  buf_request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf_request.memory = V4L2_MEMORY_MMAP;
  buf_request.count = num_buffers;

  if (xioctl(fd, VIDIOC_REQBUFS, &buf_request) == details::IOCTL_ERROR_CODE) {
    PLOG_ERROR.printf("Failed to request mmap buffers: count = %d", num_buffers);
    return std::nullopt;
  }

  if (buf_request.count != num_buffers) {
    PLOG_WARNING.printf("Buffers count adjusted by device: expected %d, got %d", num_buffers, buf_request.count);
  }

  return buf_request.count;
}

inline bool mmap_buffers(types::descriptor_t fd, std::vector<types::FrameBuffer>& buffers) {
  auto buf = v4l2_buffer{};
  buf.memory = V4L2_MEMORY_MMAP;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  for (int index = 0; index < static_cast<int>(buffers.size()); index++) {
    buf.index = index;

    if (xioctl(fd, VIDIOC_QUERYBUF, &buf) == details::IOCTL_ERROR_CODE) {
      PLOG_ERROR << "Failed to query buffer: index = " << index;
      return false;
    }

    void* ptr = mmap(nullptr, buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);

    if (ptr == MAP_FAILED) {
      PLOG_ERROR << "Failed to mmap buffer: index = " << index;
      return false;
    }

    buffers[index] = {ptr, buf.length};
  }

  return true;
}

}  // namespace lirs::tools

#endif  // LIRS_TOOLS_HPP
