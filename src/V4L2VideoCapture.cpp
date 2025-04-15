#include "lirs_ros_video_streaming/V4L2VideoCapture.hpp"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace lirs {

V4L2Capture::V4L2Capture(
  std::string device, uint32_t v4l2PixFmt, int width, int height, int frameRate, int bufferSize)
: handle_{v4l2_constants::CLOSED_HANDLE},
  imageStep_{0},
  imageSize_{0},
  device_{std::move(device)},
  isStreaming_{false} {
  params_ = {
    {CaptureParam::FRAME_WIDTH, width},
    {CaptureParam::FRAME_HEIGHT, height},
    {CaptureParam::FRAME_RATE, frameRate},
    {CaptureParam::V4L2_PIX_FMT, v4l2PixFmt},
    {CaptureParam::V4L2_BUFFERS_NUM, bufferSize}};

  handle_ = V4L2Utils::open_device(device_);  // acquire resource
}

bool V4L2Capture::IsOpened() const {
  return handle_ != v4l2_constants::CLOSED_HANDLE;
}

bool V4L2Capture::IsStreaming() const {
  return isStreaming_;
}

bool V4L2Capture::StartStreaming() {
  if (!IsOpened()) return false;  // CLOSED HANDLE ERROR

  if (IsStreaming()) return true;  // ALREADY STREAMING

  if (!checkSupportedCapabilities()) return false;  // UNSUPPORTED CAPABILITIES ERROR

  if (!negotiateFormat()) return false;  // FORMAT NEGOTIATION ERROR

  if (!negotiateFrameRate()) return false;  // UNSUPPORTED FORMAT ERROR

  if (!allocateInternalBuffers()) {
    cleanupInternalBuffers();
    return false;  // BUFFERS ALLOCATION ERROR
  }

  return enableStreaming();
}

bool V4L2Capture::StopStreaming() {
  if (!IsOpened()) {
    return false;  // CLOSED HANDLE ERROR
  }

  if (!IsStreaming()) {
    return true;  // NOT STREAMING
  }

  if (!disableSteaming()) {
    return false;  // CANNOT STOP STREAMING
  };

  cleanupInternalBuffers();

  return true;
}

bool V4L2Capture::Set(CaptureParam param, int value) {
  if (IsStreaming()) return false;  // no change of params while streaming

  // TODO (Ramil Safin): Add parameters validation.
  switch (param) {
    case CaptureParam::V4L2_BUFFERS_NUM:
      if (!V4L2Utils::is_in_range_inclusive(1, v4l2_constants::V4L2_MAX_BUFFER_SIZE, value)) {
        return false;
      }
      break;
    default:
      break;
  }
  params_[param] = value;

  return true;
}

int V4L2Capture::Get(CaptureParam param) const {
  return params_.at(param);
}

std::optional<Frame> V4L2Capture::ReadFrame() {
  if (IsStreaming()) {
    if (V4L2Utils::v4l2_is_readable(handle_)) {
      return internalReadFrame();
    }
  }
  return std::nullopt;
}

bool V4L2Capture::allocateInternalBuffers() {
  v4l2_requestbuffers requestBuffers{};
  requestBuffers.count = static_cast<uint32_t>(Get(CaptureParam::V4L2_BUFFERS_NUM));
  requestBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  requestBuffers.memory = V4L2_MEMORY_MMAP;

  // enqueue requested buffers to the driver's queue

  if (V4L2Utils::xioctl(handle_, VIDIOC_REQBUFS, &requestBuffers) == V4L2Utils::ERROR_CODE) {
    if (errno == EINVAL) {
      std::cerr << "ERROR: Device does not support memory mapping - " << strerror(errno) << '\n';
    } else {
      std::cerr << "ERROR: VIDIOC_REQBUFS - " << strerror(errno) << '\n';
    }
    return false;
  }

  if (requestBuffers.count != static_cast<uint32_t>(Get(CaptureParam::V4L2_BUFFERS_NUM))) {
    params_[CaptureParam::V4L2_BUFFERS_NUM] = requestBuffers.count;

    std::cerr << "WARNING: MappedBuffer size on " << device_ << " has changed to "
              << requestBuffers.count << '\n';
  }

  internalBuffers_.reserve(requestBuffers.count);

  v4l2_buffer buffer{};
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;

  for (buffer.index = 0u; buffer.index < requestBuffers.count; ++buffer.index) {
    if (V4L2Utils::xioctl(handle_, VIDIOC_QUERYBUF, &buffer) == V4L2Utils::ERROR_CODE) {
      std::cerr << "ERROR: VIDIOC_QUERYBUF - " << strerror(errno) << '\n';
      return false;
    }

    auto bufferLength = buffer.length;

    auto bufferData =
      mmap(nullptr, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, handle_, buffer.m.offset);

    if (bufferData == MAP_FAILED) {
      std::cerr << "ERROR: Memory Mapping has failed - " << strerror(errno) << '\n';
      return false;
    }

    internalBuffers_.emplace_back(bufferData, bufferLength);
  }

  return true;
}

void V4L2Capture::cleanupInternalBuffers() {
  if (!internalBuffers_.empty()) {
    internalBuffers_.clear();
    internalBuffers_.shrink_to_fit();

    v4l2_requestbuffers requestBuffers{};
    requestBuffers.count = uint32_t{0};
    requestBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffers.memory = V4L2_MEMORY_MMAP;

    if (V4L2Utils::xioctl(handle_, VIDIOC_REQBUFS, &requestBuffers) == V4L2Utils::ERROR_CODE) {
      std::cerr << "ERROR: Cannot cleanup allocated buffers - " << strerror(errno) << '\n';
      return;
    }
  }
}

bool V4L2Capture::enableStreaming() {
  v4l2_buffer buffer{};
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;

  for (buffer.index = 0; buffer.index < static_cast<uint32_t>(Get(CaptureParam::V4L2_BUFFERS_NUM));
       ++buffer.index) {
    if (V4L2Utils::xioctl(handle_, VIDIOC_QBUF, &buffer) == V4L2Utils::ERROR_CODE) {
      std::cerr << "ERROR: VIDIOC_QBUF  - " << strerror(errno) << '\n';
      return false;
    }
  }

  if (V4L2Utils::xioctl(handle_, VIDIOC_STREAMON, &buffer.type) == V4L2Utils::ERROR_CODE) {
    std::cerr << "ERROR: Cannot enable streaming mode - " << strerror(errno) << '\n';
    return false;
  }

  isStreaming_ = true;

  return true;
}

bool V4L2Capture::disableSteaming() {
  if (auto bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      V4L2Utils::xioctl(handle_, VIDIOC_STREAMOFF, &bufType) == V4L2Utils::ERROR_CODE) {
    std::cerr << "ERROR: Unable to stop streaming - " << strerror(errno) << '\n';
    return false;
  }

  isStreaming_ = false;

  return true;
}

bool V4L2Capture::negotiateFormat() {
  if (V4L2Utils::v4l2_try_format(
        handle_, static_cast<uint32_t>(Get(CaptureParam::V4L2_PIX_FMT)),
        Get(CaptureParam::FRAME_WIDTH), Get(CaptureParam::FRAME_HEIGHT))) {
    if (
      auto format = V4L2Utils::v4l2_set_format(
        handle_, static_cast<uint32_t>(Get(CaptureParam::V4L2_PIX_FMT)),
        Get(CaptureParam::FRAME_WIDTH), Get(CaptureParam::FRAME_HEIGHT))) {
      imageStep_ = format->fmt.pix.bytesperline;
      imageSize_ = format->fmt.pix.sizeimage;
      return true;
    }
  }

  return false;
}

bool V4L2Capture::negotiateFrameRate() {
  auto num = 1u;
  if (
    auto frameRate = V4L2Utils::v4l2_set_frame_rate(handle_, num, Get(CaptureParam::FRAME_RATE))) {
    params_[CaptureParam::FRAME_RATE] = frameRate->parm.capture.timeperframe.denominator;
    return true;
  }

  return false;
}

bool V4L2Capture::checkSupportedCapabilities() {
  // TODO (Ramil Safin): Cache queried capabilities.
  auto requiredCapabilities = uint32_t{V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING};

  if (auto caps = V4L2Utils::v4l2_query_capabilities(handle_)) {
    return V4L2Utils::v4l2_check_input_capabilities(handle_) &&
           V4L2Utils::v4l2_check_capabilities(caps.value(), requiredCapabilities);
  }

  return true;
}

std::optional<Frame> V4L2Capture::internalReadFrame() {
  v4l2_buffer buffer{};
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;

  int dequeryStatus{0};

  // dequery v4l2 buffers from the driver's outgoing queue

  while ((dequeryStatus = V4L2Utils::xioctl(handle_, VIDIOC_DQBUF, &buffer)) < 0 &&
         (errno == EINTR))
    ;

  if (dequeryStatus < 0) {
    switch (errno) {
      case EAGAIN:
        std::cerr << "WARNING: Device is not ready for reading" << strerror(errno) << '\n';
        return std::nullopt;
      case EIO:
        std::cerr << "ERROR: I/O error while reading - " << strerror(errno) << '\n';
        return std::nullopt;
      default:
        std::cerr << "ERROR: VIDIOC_DQBUF - " << strerror(errno) << '\n';
        return std::nullopt;
    }
  }

  // skip corrupted v4l2 buffers

  if (buffer.flags & V4L2_BUF_FLAG_ERROR || buffer.bytesused != static_cast<uint32_t>(imageSize_)) {
    std::cerr << "WARNING: Dequeued v4l2 buffer with size " << buffer.bytesused << '/' << imageSize_
              << " (bytes) is corrupted\n";

    buffer.bytesused = 0;

    if (V4L2Utils::xioctl(handle_, VIDIOC_QBUF, &buffer) == -1) {
      std::cerr << "ERROR: VIDIOC_QBUF - " << strerror(errno) << '\n';
    }

    return std::nullopt;
  }

  // TODO (Ramil Safin): Convert frame timestamp into absolute time.

  // copy buffer before querying it back
  Frame frame{static_cast<uint8_t*>(internalBuffers_[buffer.index].rawDataPtr), buffer.bytesused};

  if (V4L2Utils::xioctl(handle_, VIDIOC_QBUF, &buffer) == -1) {
    std::cerr << "ERROR: VIDIOC_QBUF - " << strerror(errno) << '\n';
  }

  return {frame};
}

}  // namespace lirs