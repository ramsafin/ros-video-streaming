#include "ros_video_streaming/tools.hpp"

#include <sys/mman.h>

#include <tuple>
#include <vector>

void logged_close(int fd) {
  if (fd <= -1) return;

  if (!close(fd)) {
    std::cerr << "ERROR: could not close fd: " << fd << '\n';
    return;
  }

  std::cout << "INFO: closed fd: " << fd << '\n';
}

std::tuple<char, char, char, char> fourcc(uint32_t pixel_format) {
  return std::make_tuple(
    (pixel_format >> 0) & 0xFF, (pixel_format >> 8) & 0xFF, (pixel_format >> 16) & 0xFF,
    (pixel_format >> 24) & 0xFF);
}

void frame_callback(uint8_t* data, int length) {
  std::cout << "INFO: captured frame [" << length << "] bytes\n";
}

struct FrameBuffer {
  void* data;
  size_t length;
};

int main(int argc, char const* argv[]) {
  // read/write non-blocking access
  int fd = open("/dev/video0", O_RDWR | O_NONBLOCK);

  if (fd == -1) {
    std::cerr << "ERROR: failed to open device\n";
    return EXIT_FAILURE;  // or -1 (?)
  }

  // query device capabilities
  std::optional<v4l2_capability> caps = lirs::tools::query_capabilities(fd);

  if (!caps) {
    std::cerr << "ERROR: could not query caps\n";
    return EXIT_FAILURE;
  }

  if (!(caps->capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    std::cerr << "ERROR: video capture not supported\n";
    logged_close(fd);
    return EXIT_FAILURE;
  }

  if (!(caps->capabilities & V4L2_CAP_STREAMING)) {
    std::cerr << "ERROR: streaming not supported\n";
    logged_close(fd);
    return EXIT_FAILURE;
  }

  // TBD: see the suported formats, frame size, frame intervals (VIDIOC_ENUM_*)
  // Note: check V4L2_CAP_TIMEPERFRAME before calling VIDIOC_S_PARM

  // TBD: set format, frame size, frame intervals and check with VIDIOC_G_* calls
  // Note: some VIDIOC_G_* calls fail on unsupported features

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
    char a, b, c, d;
    std::tie(a, b, c, d) = fourcc(fmt.fmt.pix.pixelformat);

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
  buf_req.count = 4;  // Number of buffers (recommended: 4-8)
  buf_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf_req.memory = V4L2_MEMORY_MMAP;  // Zero-copy mapping

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

    // wait for new frame (timeout: 2 seconds)
    struct timeval tv = {2, 0};
    int ret = select(fd + 1, &fds, NULL, NULL, &tv);

    if (ret == -1) {
      perror("WARN: select() failed");
      break;
    } else if (ret == 0) {
      std::cerr << "WARN: timeout waiting for frame\n";
      continue;
    }

    // Dequeue a filled buffer
    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (lirs::tools::xioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
      perror("ERROR: failed to dequeue buffer");
      break;
    }

    // process frames
    frame_callback(reinterpret_cast<uint8_t*>(buffers[buf.index].data), buf.bytesused);

    // Requeue the buffer
    if (lirs::tools::xioctl(fd, VIDIOC_QBUF, &buf) == -1) {
      perror("ERROR: failed to requeue buffer");
      break;
    }
  }

  // stop streaming
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
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
