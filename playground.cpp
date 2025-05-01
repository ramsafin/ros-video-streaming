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

using namespace lirs;
using namespace std::string_literals;

void frame_callback(uint8_t* data, int length) {
  std::cout << "INFO: captured frame [" << length << "] bytes\n";
}

struct VideoDevice {
  explicit VideoDevice(const std::string& name) : name_{name} {
    fd_ = tools::open_device(name);
  }

  types::descriptor_t fd() const {
    return fd_;
  }

  bool isOpen() const {
    return fd_ != -1;
  }

  ~VideoDevice() {
    tools::close_device(fd_);
    PLOG_INFO.printf("Closed video device: %s", name_.c_str());
  }

private:
  std::string name_;
  types::descriptor_t fd_{-1};
};

// TBD: namespace lirs => rvs (ros video streaming)

int main(int argc, char const* argv[]) {
  plog::init<plog::TxtFormatter>(plog::debug, plog::streamStdOut);

  // TBD: RAII video device
  const auto dev_name = "/dev/video0"s;

  if (!tools::is_character_device(dev_name)) {
    PLOG_ERROR.printf("Not a character device: %s", dev_name.c_str());
    return EXIT_FAILURE;
  }

  const auto device = VideoDevice(dev_name);

  if (!device.isOpen()) {
    PLOG_ERROR.printf("Failed to open device: %s", dev_name.c_str());
    return EXIT_FAILURE;
  }

  const types::descriptor_t fd = device.fd();

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
    return EXIT_FAILURE;
  }

  // query available formats: pixel format, resolution, frame rate
  const std::vector<v4l2_fmtdesc> pix_formats = tools::list_pixel_formats(fd);

  auto fmap = types::FormatMap{};
  fmap.reserve(pix_formats.size());

  for (const auto& pix_format : conversion::convert_pix_formats(pix_formats)) {
    const std::vector<v4l2_frmsizeenum> frame_sizes = tools::list_frame_sizes(fd, pix_format);

    for (const auto& resolution : conversion::convert_resolution(frame_sizes)) {
      const std::vector<v4l2_frmivalenum> frame_rates = tools::list_frame_rates(fd, pix_format, resolution);
      fmap[pix_format].emplace(resolution, conversion::convert_frame_rate(frame_rates));
    }
  }

  // set format
  const auto format = tools::set_format(fd, formats::PixelFormat::V4L2_YUYV, {320, 240});

  if (!format) {
    PLOG_ERROR << "Could not set format for device: " << dev_name;
    return EXIT_FAILURE;
  }

  // set framerate
  const auto framerate = tools::set_frame_rate(fd, {1, 30});

  if (!framerate) {
    PLOG_ERROR << "Could not set framerate for device: " << dev_name;
    return EXIT_FAILURE;
  }

  // request buffer
  const auto num_buffers = tools::request_mmap_buffers(fd, 32);

  if (!num_buffers || *num_buffers < 2) {
    PLOG_ERROR << "Insufficient buffer memory";
    return EXIT_FAILURE;
  }

  PLOG_INFO << "Allocated buffers (mmap): count = " << *num_buffers;

  // map buffers into user-space
  std::vector<types::FrameBuffer> buffers(*num_buffers);

  if (!tools::mmap_buffers(fd, buffers)) {
    PLOG_ERROR << "Could not mmap buffers into user-space";
    return EXIT_FAILURE;
  }

  PLOG_INFO << "Mapped buffers into user-space (mmap)";

  return EXIT_SUCCESS;

  // queue buffers for streaming
  for (int index = 0; index < *num_buffers; index++) {
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
