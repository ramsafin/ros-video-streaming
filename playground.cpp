#include "ros_video_streaming/conversion.hpp"
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

void frame_callback(uint8_t* data, int length) { std::cout << "INFO: captured frame [" << length << "] bytes\n"; }

struct FrameBuffer {
  void* data;
  size_t length;
};

using namespace lirs;

// TBD: namespace lirs => rvs (ros video streaming)

int main(int argc, char const* argv[]) {
  plog::init<plog::TxtFormatter>(plog::debug, plog::streamStdOut);

  const auto device = "/dev/video0"s;

  if (!tools::is_character_device(device)) {
    PLOG_ERROR.printf("Not a character device: %s", device.c_str());
    return EXIT_FAILURE;
  }

  // TBD: RAII video device
  const types::descriptor_t fd = tools::open_device(device);

  if (fd == -1) {
    PLOG_ERROR.printf("Failed to open device: %s", device.c_str());
    return EXIT_FAILURE;
  }

  // list and check inputs
  const std::vector<v4l2_input> inputs = tools::list_available_inputs(fd);

  PLOG_INFO.printf("Available inputs:");

  for (const auto& input : inputs) {
    const auto type = types::InputType{input.type};
    PLOG_INFO.printf("  - index: %u, name: %s, type: %s", input.index, input.name, type.name().data());
  }

  // query device capabilities
  const std::optional<v4l2_capability> caps = lirs::tools::query_capabilities(fd);

  if (!caps) {
    PLOG_ERROR << "Failed to query device capabilities";
    return EXIT_FAILURE;
  }

  PLOG_INFO << "Driver info:";
  PLOG_INFO.printf("  - Driver: %s", caps->driver);
  PLOG_INFO.printf("  - Card: %s", caps->card);
  PLOG_INFO.printf("  - Bus: %s", caps->bus_info);

  // check video streaming caps
  if (!tools::check_video_streaming_caps(caps->capabilities)) {
    PLOG_ERROR << "Device does not support streaming and video capture";
    tools::close_device(fd);
    return EXIT_FAILURE;
  }

  const std::vector<v4l2_fmtdesc> pix_formats = tools::list_pixel_formats(fd);

  // formats dictionary
  auto fmap = types::FormatMap{pix_formats.size()};

  for (const auto& pix_format : conversion::convert_pix_formats(pix_formats)) {
    const std::vector<v4l2_frmsizeenum> frame_sizes = tools::list_frame_sizes(fd, pix_format);

    for (const auto& resolution : conversion::convert_resolution(frame_sizes)) {
      const std::vector<v4l2_frmivalenum> frame_rates = tools::list_frame_rates(fd, pix_format, resolution);
      fmap[pix_format].emplace(resolution, conversion::convert_frame_rate(frame_rates));
    }
  }

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
    return EXIT_FAILURE;
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
    return EXIT_FAILURE;
  }

  if (buf_req.count < 2) {
    std::cerr << "ERROR: insufficient buffer memory\n";
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
      return EXIT_FAILURE;
    }

    // Note: check PROT_READ and PROT_WRITE
    void* buffer = mmap(nullptr, buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);

    if (buffer == MAP_FAILED) {
      perror("ERROR: failed to mmap buffer");
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
      return EXIT_FAILURE;
    }
  }

  // start streaming
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (lirs::tools::xioctl(fd, VIDIOC_STREAMON, &type) == -1) {
    perror("ERROR: failed to start streaming");
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
  tools::close_device(fd);

  return 0;
}
