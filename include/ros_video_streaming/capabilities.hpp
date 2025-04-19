#ifndef LIRS_CAPAPABILITIES_HPP
#define LIRS_CAPAPABILITIES_HPP

#include "ros_video_streaming/tools.hpp"

#include <cstdint>  // size_t, uint32_t
#include <unordered_map>
#include <utility>  // pair
#include <vector>

namespace lirs::caps
{
namespace details
{
struct pair_hash
{
  template <class T1, class T2>
  size_t operator()(const std::pair<T1, T2>& p) const
  {
    const auto h1 = std::hash<T1>{}(p.first);
    const auto h2 = std::hash<T2>{}(p.second);
    return h1 ^ (h2 << 1);
  }
};
}  // namespace details

struct Resolution
{
  uint32_t width;
  uint32_t height;
};

struct FrameRate
{
  uint32_t num;
  uint32_t den;
};

using PixelFormat = uint32_t;
using FrameRateList = std::vector<FrameRate>;
using ResolutionMap = std::unordered_map<Resolution, FrameRateList, details::pair_hash>;

using CapabilityMap = std::unordered_map<PixelFormat, ResolutionMap>;

}  // namespace lirs::caps

#endif  // LIRS_CAPAPABILITIES_HPP
