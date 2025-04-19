#include "ros_video_streaming/tools.hpp"

#include <plog/Formatters/TxtFormatter.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <plog/Log.h>

#include <fcntl.h>     // open, O_RDWR, O_NONBLOCK
#include <sys/mman.h>  // mmap, unmmap
#include <unistd.h>    // close
#include <cstdlib>     // EXIT_FAILURE

#include <string>
#include <vector>

using namespace std::string_literals;

void logged_close(int fd)
{
  if (fd <= -1) return;

  if (close(fd) != 0) {
    PLOG_WARNING << "Failed to close fd: " << fd;
    return;
  }

  PLOG_INFO.printf("Closed device (fd: %d)", fd);
}

std::array<uint8_t, 4> fourcc(uint32_t pixel_format, bool is_big_endian = false)
{
  const uint32_t mask = 0xFF;

  if (is_big_endian) {
    return {
      (pixel_format >> 24) & mask, (pixel_format >> 16) & mask, (pixel_format >> 8) & mask,
      (pixel_format)&mask};
  }

  return {
    pixel_format & mask, (pixel_format >> 8) & mask, (pixel_format >> 16) & mask,
    (pixel_format >> 24) & mask};
}

void frame_callback(uint8_t* data, int length)
{
  std::cout << "INFO: captured frame [" << length << "] bytes\n";
}

struct FrameBuffer
{
  void* data;
  size_t length;
};

int main(int argc, char const* argv[])
{
  // setup logging
  plog::init<plog::TxtFormatter>(plog::debug, plog::streamStdOut);

  // specify video device (/dev/video*)
  const auto device = "/dev/video0"s;

  // read/write non-blocking access to the device
  // TBD: create RAII wrapper for device
  PLOG_INFO.printf("Open device: %s (flags = O_RDWR, O_NONBLOCK)", device.c_str());
  const int fd = open(device.c_str(), O_RDWR | O_NONBLOCK);

  if (fd == -1) {
    PLOG_ERROR.printf("Failed to open device: %s. %s", device.c_str(), strerror(errno));
    return EXIT_FAILURE;
  }

  PLOG_INFO.printf("Opened device (fd: %d)", fd);

  // query device capabilities
  std::optional<v4l2_capability> caps = lirs::tools::query_capabilities(fd);

  if (!caps) {
    PLOG_ERROR << "Failed to query device capabilities";
    return EXIT_FAILURE;
  }

  PLOG_INFO << "Driver info:";
  PLOG_INFO.printf("  - Driver: %s", caps->driver);
  PLOG_INFO.printf("  - Card: %s", caps->card);
  PLOG_INFO.printf("  - Bus: %s", caps->bus_info);

  // check required capabilities
  if (!(caps->capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    PLOG_ERROR << "Device does not support V4L2_CAP_VIDEO_CAPTURE";
    logged_close(fd);
    return EXIT_FAILURE;
  }

  if (!(caps->capabilities & V4L2_CAP_STREAMING)) {
    PLOG_ERROR << "Device does not support V4L2_CAP_STREAMING";
    logged_close(fd);
    return EXIT_FAILURE;
  }

  // list supported pixel formats
  v4l2_fmtdesc fmtd = {};
  fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  PLOG_INFO << "Supported pixel formats:";
  for (fmtd.index = 0; lirs::tools::xioctl(fd, VIDIOC_ENUM_FMT, &fmtd) == 0; fmtd.index++) {
    PLOG_INFO << "  - " << fmtd.description;
  }

  // TBD: store the supported pixel formats in an efficient way (unordered_map?)

  // list supported frame sizes (resolution in pixels)
  v4l2_frmsizeenum frame_size = {};
  frame_size.pixel_format = V4L2_PIX_FMT_MJPEG;  // V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG

  PLOG_INFO << "Supported frame sizes:";

  for (frame_size.index = 0; lirs::tools::xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frame_size) == 0;
       frame_size.index++) {
    if (frame_size.type != V4L2_FRMSIZE_TYPE_DISCRETE) {
      PLOG_WARNING << "Continuous or stepwise framesize not handled";
      continue;
    }
    PLOG_INFO.printf("  - %d x %d", frame_size.discrete.width, frame_size.discrete.height);
  }

  // TBD: store the supported frame sizes (pixel format) in an efficient way

  // list supported frame rates (pixel format, frame size)
  v4l2_frmivalenum frmival = {};
  frmival.pixel_format = V4L2_PIX_FMT_MJPEG;
  frmival.width = 640;
  frmival.height = 480;

  PLOG_INFO << "Supported frame rates";

  for (frmival.index = 0; lirs::tools::xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0;
       frmival.index++) {
    if (!frmival.type != V4L2_FRMIVAL_TYPE_DISCRETE) {
      LOG_WARNING << "Continuous or stepwise intervals not handled";
      continue;
    }

    PLOG_INFO.printf("  - %d/%d FPS", frmival.discrete.denominator, frmival.discrete.numerator);
  }

  // TBD: store the supported frame rates (pixel format, frame size) in an efficient way
  // store the mappings pixel format - frame size - framerate (primary is pixel format, secondary is
  // frame size)

  // TBD: set format, frame size, frame intervals and check with VIDIOC_G_*
  // Note: some VIDIOC_G_* calls fail on unsupported features
  // Note: check V4L2_CAP_TIMEPERFRAME before calling VIDIOC_S_PARM

  return 0;

  // set format (resolution and pixel format)
  v4l2_format fmt = {};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = 640;
  fmt.fmt.pix.height = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;  // Progressive scan: V4L2_FIELD_NONE

  if (lirs::tools::xioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
    std::cerr << "ERROR: failed to set format\n";
    logged_close(fd);
    return EXIT_FAILURE;
  }

  if (lirs::tools::xioctl(fd, VIDIOC_G_FMT, &fmt) != -1) {
    auto [a, b, c, d] = fourcc(fmt.fmt.pix.pixelformat);

    printf(
      "INFO: selected format: %dx%d (4CC: %c%c%c%c)", fmt.fmt.pix.width, fmt.fmt.pix.height, a, b,
      c, d);
  }

  // set frame rate
  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator = 1;     // 1 second
  parm.parm.capture.timeperframe.denominator = 30;  // 30 FPS

  if (lirs::tools::xioctl(fd, VIDIOC_S_PARM, &parm) == -1) {
    std::cout << "WARN: failed to set frame rate\n";
  }

  if (lirs::tools::xioctl(fd, VIDIOC_G_PARM, &parm) != -1) {
    const auto framerate = parm.parm.capture.timeperframe.denominator;
    std::cout << "INFO: selected frame rate: " << framerate << '\n';
  }

  // request buffer (MMAP)
  v4l2_requestbuffers buf_req = {};
  buf_req.count = 4;  // number of buffers (recommended: 4-8)
  buf_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf_req.memory = V4L2_MEMORY_MMAP;

  if (lirs::tools::xioctl(fd, VIDIOC_REQBUFS, &buf_req) == -1) {
    perror("ERROR: failed to request buffers");
    logged_close(fd);
    return EXIT_FAILURE;
  }

  if (buf_req.count < 2) {
    std::cerr << "ERROR: insufficient buffer memory\n";
    logged_close(fd);
    return EXIT_FAILURE;
  }

  // map buffers into user-space
  std::vector<FrameBuffer> buffers(buf_req.count);

  for (int index = 0; index < buf_req.count; index++) {
    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;

    if (lirs::tools::xioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
      perror("ERROR: failed to query buffer");
      logged_close(fd);
      return EXIT_FAILURE;
    }

    // Note: check PROT_READ and PROT_WRITE
    void* buffer = mmap(nullptr, buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);

    if (buffer == MAP_FAILED) {
      perror("ERROR: failed to mmap buffer");
      logged_close(fd);
      return EXIT_FAILURE;
    }

    buffers[index] = {buffer, buf.length};
  }

  // queue buffers for streaming
  for (int index = 0; index < buf_req.count; index++) {
    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = index;

    if (lirs::tools::xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
      perror("ERROR: failed to queue buffer");
      logged_close(fd);
      return EXIT_FAILURE;
    }
  }

  // start streaming
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (lirs::tools::xioctl(fd, VIDIOC_STREAMON, &type) == -1) {
    perror("ERROR: failed to start streaming");
    logged_close(fd);
    return EXIT_FAILURE;
  }

  // streaming loop (capture frames)

  bool streaming = true;

  while (streaming) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // wait for new frame (timeout: 1 seconds)
    struct timeval tv = {1, 0};
    int ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

    if (ret == -1) {
      perror("WARN: select() failed");
      break;
    } else if (ret == 0) {
      std::cerr << "WARN: timeout waiting for frame\n";
      continue;
    }

    // dequeue a filled buffer
    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (lirs::tools::xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
      perror("ERROR: failed to dequeue buffer");
      break;
    }

    // process frames
    frame_callback(reinterpret_cast<uint8_t*>(buffers[buf.index].data), buf.bytesused);

    // requeue the buffer
    if (lirs::tools::xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
      perror("ERROR: failed to requeue buffer");
      break;
    }
  }

  // stop streaming
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (lirs::tools::xioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
    std::cerr << "WARN: failed to stop streaming\n";
  }

  // unmap buffers
  for (auto& buf : buffers) {
    munmap(buf.data, buf.length);
  }

  // close device
  logged_close(fd);

  return 0;
}
