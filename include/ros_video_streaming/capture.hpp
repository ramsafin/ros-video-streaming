#pragma once

#include "ros_video_streaming/frame.hpp"
#include "ros_video_streaming/tools.hpp"

#include <linux/videodev2.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace lirs
{
class VideoCapture
{
public:
  VideoCapture(std::string device) : device_{std::move(device)}
  {
    handle_ = tools::open_device(device_);

    if (!IsOpened()) {
      std::cerr << "ERROR: Device is not initialized\n";
      return;
    }

    // fix the pipeline
    // 1. query / get formats, controls, streaming parameters, etc.
    // 2..
  }

  virtual ~VideoCapture()
  {
    if (!IsOpened()) return;
    if (IsStreaming()) StopStreaming();

    tools::close_device(handle_);
  };

  // Device state
  virtual bool IsOpened() const
  {
    return handle_ != tools::CLOSED_HANDLE;
  };

  virtual bool IsStreaming() const
  {
    return is_streaming_.load(std::memory_order_relaxed);
  };

  // Stream control
  virtual bool StartStreaming()
  {
    if (!IsOpened()) return false;
    if (IsStreaming()) return true;
    if (!checkCapabilities()) return false;
    if (!setupFormat()) return false;
    //
  }

  virtual bool StopStreaming();

  // Frame acquisition
  virtual std::optional<Frame> Read();

  // Device information
  virtual const std::string& device() const;

  // Non-copyable and non-movable
  VideoCapture(const VideoCapture&) = delete;
  VideoCapture& operator=(const VideoCapture&) = delete;

  VideoCapture(VideoCapture&&) = delete;
  VideoCapture& operator=(VideoCapture&&) = delete;

private:
  std::string device_;
  v4l2_format format_;
  v4l2_captureparm params_;
  int handle_{tools::CLOSED_HANDLE};

  mutable std::mutex mtx_;
  std::atomic_bool is_streaming_{false};

  // internals
  bool checkCapabilities() const
  {
    if (!tools::check_input_capabilities(handle_)) {
      return false;
    }

    const uint32_t required_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;

    if (auto caps = tools::query_capabilities(handle_)) {
      return tools::validate_capabilities(*caps, required_caps);
    }

    return false;  // failed to query caps
  }

  bool setupFormat()
  {
    if (auto format = tools::set_format(handle_, V4L2_PIX_FMT_MJPEG, 640, 480)) {
      format_ = *format;
      return true;
    }

    return false;  // failed to set format
  }

  bool setupFramerate()
  {
    if (auto framerate = tools::set_frame_rate(handle_, 1, 30)) {
      return true;
    }
  }
};

}  // namespace lirs
