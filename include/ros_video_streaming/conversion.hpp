#ifndef LIRS_CONVERSION_HPP
#define LIRS_CONVERSION_HPP

#include <algorithm>
#include <vector>

#include "ros_video_streaming/types.hpp"

namespace lirs::conversion {

inline types::FrameRate convert_frame_rate(const v4l2_frmivalenum& frame_rate) {
  return types::FrameRate{frame_rate.discrete.numerator, frame_rate.discrete.denominator};
}

inline types::FrameRateList convert_frame_rate(const std::vector<v4l2_frmivalenum>& frame_rates) {
  auto frame_rate_list = types::FrameRateList{};
  frame_rate_list.reserve(frame_rates.size());

  const auto converter = [](const v4l2_frmivalenum& fps) { return convert_frame_rate(fps); };

  std::transform(std::begin(frame_rates), std::end(frame_rates), std::back_inserter(frame_rate_list), converter);

  return frame_rate_list;
}

inline types::Resolution convert_resolution(const v4l2_frmsizeenum& resolution) {
  return types::Resolution{resolution.discrete.width, resolution.discrete.height};
}

inline types::ResolutionList convert_resolution(const std::vector<v4l2_frmsizeenum>& resolutions) {
  auto resolution_list = types::ResolutionList{};
  resolution_list.reserve(resolutions.size());

  const auto converter = [](const v4l2_frmsizeenum& resolution) { return convert_resolution(resolution); };

  std::transform(std::begin(resolutions), std::end(resolutions), std::back_inserter(resolution_list), converter);

  return resolution_list;
}

inline std::vector<types::pix_format_t> convert_pix_formats(const std::vector<v4l2_fmtdesc>& pix_formats) {
  auto pix_format_list = types::PixFormatList{};
  pix_format_list.reserve(pix_formats.size());

  const auto converter = [](const v4l2_fmtdesc& fmt) { return fmt.pixelformat; };

  std::transform(std::begin(pix_formats), std::end(pix_formats), std::back_inserter(pix_format_list), converter);

  return pix_format_list;
}

}  // namespace lirs::conversion

#endif  // LIRS_CONVERSION_HPP
