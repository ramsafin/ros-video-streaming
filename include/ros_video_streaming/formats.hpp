#pragma once

#include <linux/videodev2.h>

#include <cstdint>
#include <optional>
#include <string_view>

namespace lirs::formats {

enum class PixelFormat : uint32_t {
  V4L2_RGB1 = V4L2_PIX_FMT_RGB332,
  V4L2_R444 = V4L2_PIX_FMT_RGB444,
  V4L2_AR12 = V4L2_PIX_FMT_ARGB444,
  V4L2_XR12 = V4L2_PIX_FMT_XRGB444,
  V4L2_RA12 = V4L2_PIX_FMT_RGBA444,
  V4L2_RX12 = V4L2_PIX_FMT_RGBX444,
  V4L2_AB12 = V4L2_PIX_FMT_ABGR444,
  V4L2_XB12 = V4L2_PIX_FMT_XBGR444,
  V4L2_GA12 = V4L2_PIX_FMT_BGRA444,
  V4L2_BX12 = V4L2_PIX_FMT_BGRX444,
  V4L2_RGBO = V4L2_PIX_FMT_RGB555,
  V4L2_AR15 = V4L2_PIX_FMT_ARGB555,
  V4L2_XR15 = V4L2_PIX_FMT_XRGB555,
  V4L2_RA15 = V4L2_PIX_FMT_RGBA555,
  V4L2_RX15 = V4L2_PIX_FMT_RGBX555,
  V4L2_AB15 = V4L2_PIX_FMT_ABGR555,
  V4L2_XB15 = V4L2_PIX_FMT_XBGR555,
  V4L2_BA15 = V4L2_PIX_FMT_BGRA555,
  V4L2_BX15 = V4L2_PIX_FMT_BGRX555,
  V4L2_RGBP = V4L2_PIX_FMT_RGB565,
  V4L2_RGBQ = V4L2_PIX_FMT_RGB555X,
  V4L2_RGBR = V4L2_PIX_FMT_RGB565X,
  V4L2_BGRH = V4L2_PIX_FMT_BGR666,
  V4L2_BGR3 = V4L2_PIX_FMT_BGR24,
  V4L2_RGB3 = V4L2_PIX_FMT_RGB24,
  V4L2_BGR4 = V4L2_PIX_FMT_BGR32,
  V4L2_AR24 = V4L2_PIX_FMT_ABGR32,
  V4L2_XR24 = V4L2_PIX_FMT_XBGR32,
  V4L2_RA24 = V4L2_PIX_FMT_BGRA32,
  V4L2_RX24 = V4L2_PIX_FMT_BGRX32,
  V4L2_RGB4 = V4L2_PIX_FMT_RGB32,
  V4L2_AB24 = V4L2_PIX_FMT_RGBA32,
  V4L2_XB24 = V4L2_PIX_FMT_RGBX32,
  V4L2_BA24 = V4L2_PIX_FMT_ARGB32,
  V4L2_BX24 = V4L2_PIX_FMT_XRGB32,
  V4L2_GREY = V4L2_PIX_FMT_GREY,
  V4L2_Y04 = V4L2_PIX_FMT_Y4,
  V4L2_Y06 = V4L2_PIX_FMT_Y6,
  V4L2_Y10 = V4L2_PIX_FMT_Y10,
  V4L2_Y12 = V4L2_PIX_FMT_Y12,
  V4L2_Y16 = V4L2_PIX_FMT_Y16,
  V4L2_Y10B = V4L2_PIX_FMT_Y10BPACK,
  V4L2_Y10P = V4L2_PIX_FMT_Y10P,
  V4L2_PAL8 = V4L2_PIX_FMT_PAL8,
  V4L2_UV8 = V4L2_PIX_FMT_UV8,
  V4L2_YUYV = V4L2_PIX_FMT_YUYV,
  V4L2_YYUV = V4L2_PIX_FMT_YYUV,
  V4L2_YVYU = V4L2_PIX_FMT_YVYU,
  V4L2_UYVY = V4L2_PIX_FMT_UYVY,
  V4L2_VYUY = V4L2_PIX_FMT_VYUY,
  V4L2_Y41P = V4L2_PIX_FMT_Y41P,
  V4L2_Y444 = V4L2_PIX_FMT_YUV444,
  V4L2_YUVO = V4L2_PIX_FMT_YUV555,
  V4L2_YUVP = V4L2_PIX_FMT_YUV565,
  V4L2_YUV4 = V4L2_PIX_FMT_YUV32,
  V4L2_AYUV = V4L2_PIX_FMT_AYUV32,
  V4L2_XYUV = V4L2_PIX_FMT_XYUV32,
  V4L2_VUYA = V4L2_PIX_FMT_VUYA32,
  V4L2_VUYX = V4L2_PIX_FMT_VUYX32,
  V4L2_HI24 = V4L2_PIX_FMT_HI240,
  V4L2_HM12 = V4L2_PIX_FMT_HM12,
  V4L2_M420 = V4L2_PIX_FMT_M420,
  V4L2_NV12 = V4L2_PIX_FMT_NV12,
  V4L2_NV21 = V4L2_PIX_FMT_NV21,
  V4L2_NV16 = V4L2_PIX_FMT_NV16,
  V4L2_NV61 = V4L2_PIX_FMT_NV61,
  V4L2_NV24 = V4L2_PIX_FMT_NV24,
  V4L2_NV42 = V4L2_PIX_FMT_NV42,
  V4L2_NM12 = V4L2_PIX_FMT_NV12M,
  V4L2_NM21 = V4L2_PIX_FMT_NV21M,
  V4L2_NM16 = V4L2_PIX_FMT_NV16M,
  V4L2_NM61 = V4L2_PIX_FMT_NV61M,
  V4L2_TM12 = V4L2_PIX_FMT_NV12MT,
  V4L2_VM12 = V4L2_PIX_FMT_NV12MT_16X16,
  V4L2_YUV9 = V4L2_PIX_FMT_YUV410,
  V4L2_YVU9 = V4L2_PIX_FMT_YVU410,
  V4L2_411P = V4L2_PIX_FMT_YUV411P,
  V4L2_YU12 = V4L2_PIX_FMT_YUV420,
  V4L2_YV12 = V4L2_PIX_FMT_YVU420,
  V4L2_422P = V4L2_PIX_FMT_YUV422P,
  V4L2_YM12 = V4L2_PIX_FMT_YUV420M,
  V4L2_YM21 = V4L2_PIX_FMT_YVU420M,
  V4L2_YM16 = V4L2_PIX_FMT_YUV422M,
  V4L2_YM61 = V4L2_PIX_FMT_YVU422M,
  V4L2_YM24 = V4L2_PIX_FMT_YUV444M,
  V4L2_YM42 = V4L2_PIX_FMT_YVU444M,
  V4L2_BA81 = V4L2_PIX_FMT_SBGGR8,
  V4L2_GBRG = V4L2_PIX_FMT_SGBRG8,
  V4L2_GRBG = V4L2_PIX_FMT_SGRBG8,
  V4L2_RGGB = V4L2_PIX_FMT_SRGGB8,
  V4L2_BG10 = V4L2_PIX_FMT_SBGGR10,
  V4L2_GB10 = V4L2_PIX_FMT_SGBRG10,
  V4L2_BA10 = V4L2_PIX_FMT_SGRBG10,
  V4L2_RG10 = V4L2_PIX_FMT_SRGGB10,
  V4L2_pBAA = V4L2_PIX_FMT_SBGGR10P,
  V4L2_pGAA = V4L2_PIX_FMT_SGBRG10P,
  V4L2_pgAA = V4L2_PIX_FMT_SGRBG10P,
  V4L2_pRAA = V4L2_PIX_FMT_SRGGB10P,
  V4L2_aBA8 = V4L2_PIX_FMT_SBGGR10ALAW8,
  V4L2_aGA8 = V4L2_PIX_FMT_SGBRG10ALAW8,
  V4L2_agA8 = V4L2_PIX_FMT_SGRBG10ALAW8,
  V4L2_aRA8 = V4L2_PIX_FMT_SRGGB10ALAW8,
  V4L2_bBA8 = V4L2_PIX_FMT_SBGGR10DPCM8,
  V4L2_bGA8 = V4L2_PIX_FMT_SGBRG10DPCM8,
  V4L2_BD10 = V4L2_PIX_FMT_SGRBG10DPCM8,
  V4L2_bRA8 = V4L2_PIX_FMT_SRGGB10DPCM8,
  V4L2_BG12 = V4L2_PIX_FMT_SBGGR12,
  V4L2_GB12 = V4L2_PIX_FMT_SGBRG12,
  V4L2_BA12 = V4L2_PIX_FMT_SGRBG12,
  V4L2_RG12 = V4L2_PIX_FMT_SRGGB12,
  V4L2_pBCC = V4L2_PIX_FMT_SBGGR12P,
  V4L2_pGCC = V4L2_PIX_FMT_SGBRG12P,
  V4L2_pgCC = V4L2_PIX_FMT_SGRBG12P,
  V4L2_pRCC = V4L2_PIX_FMT_SRGGB12P,
  V4L2_pBEE = V4L2_PIX_FMT_SBGGR14P,
  V4L2_pGEE = V4L2_PIX_FMT_SGBRG14P,
  V4L2_pgEE = V4L2_PIX_FMT_SGRBG14P,
  V4L2_pREE = V4L2_PIX_FMT_SRGGB14P,
  V4L2_BYR2 = V4L2_PIX_FMT_SBGGR16,
  V4L2_GB16 = V4L2_PIX_FMT_SGBRG16,
  V4L2_GR16 = V4L2_PIX_FMT_SGRBG16,
  V4L2_RG16 = V4L2_PIX_FMT_SRGGB16,
  V4L2_HSV3 = V4L2_PIX_FMT_HSV24,
  V4L2_HSV4 = V4L2_PIX_FMT_HSV32,
  V4L2_MJPG = V4L2_PIX_FMT_MJPEG,
  V4L2_JPEG = V4L2_PIX_FMT_JPEG,
  V4L2_dvsd = V4L2_PIX_FMT_DV,
  V4L2_MPEG = V4L2_PIX_FMT_MPEG,
  V4L2_H264 = V4L2_PIX_FMT_H264,
  V4L2_AVC1 = V4L2_PIX_FMT_H264_NO_SC,
  V4L2_M264 = V4L2_PIX_FMT_H264_MVC,
  V4L2_H263 = V4L2_PIX_FMT_H263,
  V4L2_MPG1 = V4L2_PIX_FMT_MPEG1,
  V4L2_MPG2 = V4L2_PIX_FMT_MPEG2,
  V4L2_MG2S = V4L2_PIX_FMT_MPEG2_SLICE,
  V4L2_MPG4 = V4L2_PIX_FMT_MPEG4,
  V4L2_XVID = V4L2_PIX_FMT_XVID,
  V4L2_VC1G = V4L2_PIX_FMT_VC1_ANNEX_G,
  V4L2_VC1L = V4L2_PIX_FMT_VC1_ANNEX_L,
  V4L2_VP80 = V4L2_PIX_FMT_VP8,
  V4L2_VP90 = V4L2_PIX_FMT_VP9,
  V4L2_HEVC = V4L2_PIX_FMT_HEVC,
  V4L2_FWHT = V4L2_PIX_FMT_FWHT,
  V4L2_SFWH = V4L2_PIX_FMT_FWHT_STATELESS,
  V4L2_CPIA = V4L2_PIX_FMT_CPIA1,
  V4L2_WNVA = V4L2_PIX_FMT_WNVA,
  V4L2_S910 = V4L2_PIX_FMT_SN9C10X,
  V4L2_S920 = V4L2_PIX_FMT_SN9C20X_I420,
  V4L2_PWC1 = V4L2_PIX_FMT_PWC1,
  V4L2_PWC2 = V4L2_PIX_FMT_PWC2,
  V4L2_E625 = V4L2_PIX_FMT_ET61X251,
  V4L2_S501 = V4L2_PIX_FMT_SPCA501,
  V4L2_S505 = V4L2_PIX_FMT_SPCA505,
  V4L2_S508 = V4L2_PIX_FMT_SPCA508,
  V4L2_S561 = V4L2_PIX_FMT_SPCA561,
  V4L2_P207 = V4L2_PIX_FMT_PAC207,
  V4L2_M310 = V4L2_PIX_FMT_MR97310A,
  V4L2_JL20 = V4L2_PIX_FMT_JL2005BCD,
  V4L2_SONX = V4L2_PIX_FMT_SN9C2028,
  V4L2_905C = V4L2_PIX_FMT_SQ905C,
  V4L2_PJPG = V4L2_PIX_FMT_PJPG,
  V4L2_O511 = V4L2_PIX_FMT_OV511,
  V4L2_O518 = V4L2_PIX_FMT_OV518,
  V4L2_S680 = V4L2_PIX_FMT_STV0680,
  V4L2_TM60 = V4L2_PIX_FMT_TM6000,
  V4L2_CITV = V4L2_PIX_FMT_CIT_YYVYUY,
  V4L2_KONI = V4L2_PIX_FMT_KONICA420,
  V4L2_JPGL = V4L2_PIX_FMT_JPGL,
  V4L2_S401 = V4L2_PIX_FMT_SE401,
  V4L2_S5CI = V4L2_PIX_FMT_S5C_UYVY_JPG,
  V4L2_Y8I = V4L2_PIX_FMT_Y8I,
  V4L2_Y12I = V4L2_PIX_FMT_Y12I,
  V4L2_Z16 = V4L2_PIX_FMT_Z16,
  V4L2_MT21 = V4L2_PIX_FMT_MT21C,
  V4L2_INZI = V4L2_PIX_FMT_INZI,
  V4L2_ST12 = V4L2_PIX_FMT_SUNXI_TILED_NV12,
  V4L2_CNF4 = V4L2_PIX_FMT_CNF4,
  V4L2_ip3b = V4L2_PIX_FMT_IPU3_SBGGR10,
  V4L2_ip3g = V4L2_PIX_FMT_IPU3_SGBRG10,
  V4L2_ip3G = V4L2_PIX_FMT_IPU3_SGRBG10,
  V4L2_ip3r = V4L2_PIX_FMT_IPU3_SRGGB10,
};

inline constexpr std::string_view format2str(PixelFormat fmt) {
  switch (fmt) {
    case PixelFormat::V4L2_RGB1:
      return "V4L2_PIX_FMT_RGB332";
    case PixelFormat::V4L2_R444:
      return "V4L2_PIX_FMT_RGB444";
    case PixelFormat::V4L2_AR12:
      return "V4L2_PIX_FMT_ARGB444";
    case PixelFormat::V4L2_XR12:
      return "V4L2_PIX_FMT_XRGB444";
    case PixelFormat::V4L2_RA12:
      return "V4L2_PIX_FMT_RGBA444";
    case PixelFormat::V4L2_RX12:
      return "V4L2_PIX_FMT_RGBX444";
    case PixelFormat::V4L2_AB12:
      return "V4L2_PIX_FMT_ABGR444";
    case PixelFormat::V4L2_XB12:
      return "V4L2_PIX_FMT_XBGR444";
    case PixelFormat::V4L2_GA12:
      return "V4L2_PIX_FMT_BGRA444";
    case PixelFormat::V4L2_BX12:
      return "V4L2_PIX_FMT_BGRX444";
    case PixelFormat::V4L2_RGBO:
      return "V4L2_PIX_FMT_RGB555";
    case PixelFormat::V4L2_AR15:
      return "V4L2_PIX_FMT_ARGB555";
    case PixelFormat::V4L2_XR15:
      return "V4L2_PIX_FMT_XRGB555";
    case PixelFormat::V4L2_RA15:
      return "V4L2_PIX_FMT_RGBA555";
    case PixelFormat::V4L2_RX15:
      return "V4L2_PIX_FMT_RGBX555";
    case PixelFormat::V4L2_AB15:
      return "V4L2_PIX_FMT_ABGR555";
    case PixelFormat::V4L2_XB15:
      return "V4L2_PIX_FMT_XBGR555";
    case PixelFormat::V4L2_BA15:
      return "V4L2_PIX_FMT_BGRA555";
    case PixelFormat::V4L2_BX15:
      return "V4L2_PIX_FMT_BGRX555";
    case PixelFormat::V4L2_RGBP:
      return "V4L2_PIX_FMT_RGB565";
    case PixelFormat::V4L2_RGBQ:
      return "V4L2_PIX_FMT_RGB555X";
    case PixelFormat::V4L2_RGBR:
      return "V4L2_PIX_FMT_RGB565X";
    case PixelFormat::V4L2_BGRH:
      return "V4L2_PIX_FMT_BGR666";
    case PixelFormat::V4L2_BGR3:
      return "V4L2_PIX_FMT_BGR24";
    case PixelFormat::V4L2_RGB3:
      return "V4L2_PIX_FMT_RGB24";
    case PixelFormat::V4L2_BGR4:
      return "V4L2_PIX_FMT_BGR32";
    case PixelFormat::V4L2_AR24:
      return "V4L2_PIX_FMT_ABGR32";
    case PixelFormat::V4L2_XR24:
      return "V4L2_PIX_FMT_XBGR32";
    case PixelFormat::V4L2_RA24:
      return "V4L2_PIX_FMT_BGRA32";
    case PixelFormat::V4L2_RX24:
      return "V4L2_PIX_FMT_BGRX32";
    case PixelFormat::V4L2_RGB4:
      return "V4L2_PIX_FMT_RGB32";
    case PixelFormat::V4L2_AB24:
      return "V4L2_PIX_FMT_RGBA32";
    case PixelFormat::V4L2_XB24:
      return "V4L2_PIX_FMT_RGBX32";
    case PixelFormat::V4L2_BA24:
      return "V4L2_PIX_FMT_ARGB32";
    case PixelFormat::V4L2_BX24:
      return "V4L2_PIX_FMT_XRGB32";
    case PixelFormat::V4L2_GREY:
      return "V4L2_PIX_FMT_GREY";
    case PixelFormat::V4L2_Y04:
      return "V4L2_PIX_FMT_Y4";
    case PixelFormat::V4L2_Y06:
      return "V4L2_PIX_FMT_Y6";
    case PixelFormat::V4L2_Y10:
      return "V4L2_PIX_FMT_Y10";
    case PixelFormat::V4L2_Y12:
      return "V4L2_PIX_FMT_Y12";
    case PixelFormat::V4L2_Y16:
      return "V4L2_PIX_FMT_Y16";
    case PixelFormat::V4L2_Y10B:
      return "V4L2_PIX_FMT_Y10BPACK";
    case PixelFormat::V4L2_Y10P:
      return "V4L2_PIX_FMT_Y10P";
    case PixelFormat::V4L2_PAL8:
      return "V4L2_PIX_FMT_PAL8";
    case PixelFormat::V4L2_UV8:
      return "V4L2_PIX_FMT_UV8";
    case PixelFormat::V4L2_YUYV:
      return "V4L2_PIX_FMT_YUYV";
    case PixelFormat::V4L2_YYUV:
      return "V4L2_PIX_FMT_YYUV";
    case PixelFormat::V4L2_YVYU:
      return "V4L2_PIX_FMT_YVYU";
    case PixelFormat::V4L2_UYVY:
      return "V4L2_PIX_FMT_UYVY";
    case PixelFormat::V4L2_VYUY:
      return "V4L2_PIX_FMT_VYUY";
    case PixelFormat::V4L2_Y41P:
      return "V4L2_PIX_FMT_Y41P";
    case PixelFormat::V4L2_Y444:
      return "V4L2_PIX_FMT_YUV444";
    case PixelFormat::V4L2_YUVO:
      return "V4L2_PIX_FMT_YUV555";
    case PixelFormat::V4L2_YUVP:
      return "V4L2_PIX_FMT_YUV565";
    case PixelFormat::V4L2_YUV4:
      return "V4L2_PIX_FMT_YUV32";
    case PixelFormat::V4L2_AYUV:
      return "V4L2_PIX_FMT_AYUV32";
    case PixelFormat::V4L2_XYUV:
      return "V4L2_PIX_FMT_XYUV32";
    case PixelFormat::V4L2_VUYA:
      return "V4L2_PIX_FMT_VUYA32";
    case PixelFormat::V4L2_VUYX:
      return "V4L2_PIX_FMT_VUYX32";
    case PixelFormat::V4L2_HI24:
      return "V4L2_PIX_FMT_HI240";
    case PixelFormat::V4L2_HM12:
      return "V4L2_PIX_FMT_HM12";
    case PixelFormat::V4L2_M420:
      return "V4L2_PIX_FMT_M420";
    case PixelFormat::V4L2_NV12:
      return "V4L2_PIX_FMT_NV12";
    case PixelFormat::V4L2_NV21:
      return "V4L2_PIX_FMT_NV21";
    case PixelFormat::V4L2_NV16:
      return "V4L2_PIX_FMT_NV16";
    case PixelFormat::V4L2_NV61:
      return "V4L2_PIX_FMT_NV61";
    case PixelFormat::V4L2_NV24:
      return "V4L2_PIX_FMT_NV24";
    case PixelFormat::V4L2_NV42:
      return "V4L2_PIX_FMT_NV42";
    case PixelFormat::V4L2_NM12:
      return "V4L2_PIX_FMT_NV12M";
    case PixelFormat::V4L2_NM21:
      return "V4L2_PIX_FMT_NV21M";
    case PixelFormat::V4L2_NM16:
      return "V4L2_PIX_FMT_NV16M";
    case PixelFormat::V4L2_NM61:
      return "V4L2_PIX_FMT_NV61M";
    case PixelFormat::V4L2_TM12:
      return "V4L2_PIX_FMT_NV12MT";
    case PixelFormat::V4L2_VM12:
      return "V4L2_PIX_FMT_NV12MT_16X16";
    case PixelFormat::V4L2_YUV9:
      return "V4L2_PIX_FMT_YUV410";
    case PixelFormat::V4L2_YVU9:
      return "V4L2_PIX_FMT_YVU410";
    case PixelFormat::V4L2_411P:
      return "V4L2_PIX_FMT_YUV411P";
    case PixelFormat::V4L2_YU12:
      return "V4L2_PIX_FMT_YUV420";
    case PixelFormat::V4L2_YV12:
      return "V4L2_PIX_FMT_YVU420";
    case PixelFormat::V4L2_422P:
      return "V4L2_PIX_FMT_YUV422P";
    case PixelFormat::V4L2_YM12:
      return "V4L2_PIX_FMT_YUV420M";
    case PixelFormat::V4L2_YM21:
      return "V4L2_PIX_FMT_YVU420M";
    case PixelFormat::V4L2_YM16:
      return "V4L2_PIX_FMT_YUV422M";
    case PixelFormat::V4L2_YM61:
      return "V4L2_PIX_FMT_YVU422M";
    case PixelFormat::V4L2_YM24:
      return "V4L2_PIX_FMT_YUV444M";
    case PixelFormat::V4L2_YM42:
      return "V4L2_PIX_FMT_YVU444M";
    case PixelFormat::V4L2_BA81:
      return "V4L2_PIX_FMT_SBGGR8";
    case PixelFormat::V4L2_GBRG:
      return "V4L2_PIX_FMT_SGBRG8";
    case PixelFormat::V4L2_GRBG:
      return "V4L2_PIX_FMT_SGRBG8";
    case PixelFormat::V4L2_RGGB:
      return "V4L2_PIX_FMT_SRGGB8";
    case PixelFormat::V4L2_BG10:
      return "V4L2_PIX_FMT_SBGGR10";
    case PixelFormat::V4L2_GB10:
      return "V4L2_PIX_FMT_SGBRG10";
    case PixelFormat::V4L2_BA10:
      return "V4L2_PIX_FMT_SGRBG10";
    case PixelFormat::V4L2_RG10:
      return "V4L2_PIX_FMT_SRGGB10";
    case PixelFormat::V4L2_pBAA:
      return "V4L2_PIX_FMT_SBGGR10P";
    case PixelFormat::V4L2_pGAA:
      return "V4L2_PIX_FMT_SGBRG10P";
    case PixelFormat::V4L2_pgAA:
      return "V4L2_PIX_FMT_SGRBG10P";
    case PixelFormat::V4L2_pRAA:
      return "V4L2_PIX_FMT_SRGGB10P";
    case PixelFormat::V4L2_aBA8:
      return "V4L2_PIX_FMT_SBGGR10ALAW8";
    case PixelFormat::V4L2_aGA8:
      return "V4L2_PIX_FMT_SGBRG10ALAW8";
    case PixelFormat::V4L2_agA8:
      return "V4L2_PIX_FMT_SGRBG10ALAW8";
    case PixelFormat::V4L2_aRA8:
      return "V4L2_PIX_FMT_SRGGB10ALAW8";
    case PixelFormat::V4L2_bBA8:
      return "V4L2_PIX_FMT_SBGGR10DPCM8";
    case PixelFormat::V4L2_bGA8:
      return "V4L2_PIX_FMT_SGBRG10DPCM8";
    case PixelFormat::V4L2_BD10:
      return "V4L2_PIX_FMT_SGRBG10DPCM8";
    case PixelFormat::V4L2_bRA8:
      return "V4L2_PIX_FMT_SRGGB10DPCM8";
    case PixelFormat::V4L2_BG12:
      return "V4L2_PIX_FMT_SBGGR12";
    case PixelFormat::V4L2_GB12:
      return "V4L2_PIX_FMT_SGBRG12";
    case PixelFormat::V4L2_BA12:
      return "V4L2_PIX_FMT_SGRBG12";
    case PixelFormat::V4L2_RG12:
      return "V4L2_PIX_FMT_SRGGB12";
    case PixelFormat::V4L2_pBCC:
      return "V4L2_PIX_FMT_SBGGR12P";
    case PixelFormat::V4L2_pGCC:
      return "V4L2_PIX_FMT_SGBRG12P";
    case PixelFormat::V4L2_pgCC:
      return "V4L2_PIX_FMT_SGRBG12P";
    case PixelFormat::V4L2_pRCC:
      return "V4L2_PIX_FMT_SRGGB12P";
    case PixelFormat::V4L2_pBEE:
      return "V4L2_PIX_FMT_SBGGR14P";
    case PixelFormat::V4L2_pGEE:
      return "V4L2_PIX_FMT_SGBRG14P";
    case PixelFormat::V4L2_pgEE:
      return "V4L2_PIX_FMT_SGRBG14P";
    case PixelFormat::V4L2_pREE:
      return "V4L2_PIX_FMT_SRGGB14P";
    case PixelFormat::V4L2_BYR2:
      return "V4L2_PIX_FMT_SBGGR16";
    case PixelFormat::V4L2_GB16:
      return "V4L2_PIX_FMT_SGBRG16";
    case PixelFormat::V4L2_GR16:
      return "V4L2_PIX_FMT_SGRBG16";
    case PixelFormat::V4L2_RG16:
      return "V4L2_PIX_FMT_SRGGB16";
    case PixelFormat::V4L2_HSV3:
      return "V4L2_PIX_FMT_HSV24";
    case PixelFormat::V4L2_HSV4:
      return "V4L2_PIX_FMT_HSV32";
    case PixelFormat::V4L2_MJPG:
      return "V4L2_PIX_FMT_MJPEG";
    case PixelFormat::V4L2_JPEG:
      return "V4L2_PIX_FMT_JPEG";
    case PixelFormat::V4L2_dvsd:
      return "V4L2_PIX_FMT_DV";
    case PixelFormat::V4L2_MPEG:
      return "V4L2_PIX_FMT_MPEG";
    case PixelFormat::V4L2_H264:
      return "V4L2_PIX_FMT_H264";
    case PixelFormat::V4L2_AVC1:
      return "V4L2_PIX_FMT_H264_NO_SC";
    case PixelFormat::V4L2_M264:
      return "V4L2_PIX_FMT_H264_MVC";
    case PixelFormat::V4L2_H263:
      return "V4L2_PIX_FMT_H263";
    case PixelFormat::V4L2_MPG1:
      return "V4L2_PIX_FMT_MPEG1";
    case PixelFormat::V4L2_MPG2:
      return "V4L2_PIX_FMT_MPEG2";
    case PixelFormat::V4L2_MG2S:
      return "V4L2_PIX_FMT_MPEG2_SLICE";
    case PixelFormat::V4L2_MPG4:
      return "V4L2_PIX_FMT_MPEG4";
    case PixelFormat::V4L2_XVID:
      return "V4L2_PIX_FMT_XVID";
    case PixelFormat::V4L2_VC1G:
      return "V4L2_PIX_FMT_VC1_ANNEX_G";
    case PixelFormat::V4L2_VC1L:
      return "V4L2_PIX_FMT_VC1_ANNEX_L";
    case PixelFormat::V4L2_VP80:
      return "V4L2_PIX_FMT_VP8";
    case PixelFormat::V4L2_VP90:
      return "V4L2_PIX_FMT_VP9";
    case PixelFormat::V4L2_HEVC:
      return "V4L2_PIX_FMT_HEVC";
    case PixelFormat::V4L2_FWHT:
      return "V4L2_PIX_FMT_FWHT";
    case PixelFormat::V4L2_SFWH:
      return "V4L2_PIX_FMT_FWHT_STATELESS";
    case PixelFormat::V4L2_CPIA:
      return "V4L2_PIX_FMT_CPIA1";
    case PixelFormat::V4L2_WNVA:
      return "V4L2_PIX_FMT_WNVA";
    case PixelFormat::V4L2_S910:
      return "V4L2_PIX_FMT_SN9C10X";
    case PixelFormat::V4L2_S920:
      return "V4L2_PIX_FMT_SN9C20X_I420";
    case PixelFormat::V4L2_PWC1:
      return "V4L2_PIX_FMT_PWC1";
    case PixelFormat::V4L2_PWC2:
      return "V4L2_PIX_FMT_PWC2";
    case PixelFormat::V4L2_E625:
      return "V4L2_PIX_FMT_ET61X251";
    case PixelFormat::V4L2_S501:
      return "V4L2_PIX_FMT_SPCA501";
    case PixelFormat::V4L2_S505:
      return "V4L2_PIX_FMT_SPCA505";
    case PixelFormat::V4L2_S508:
      return "V4L2_PIX_FMT_SPCA508";
    case PixelFormat::V4L2_S561:
      return "V4L2_PIX_FMT_SPCA561";
    case PixelFormat::V4L2_P207:
      return "V4L2_PIX_FMT_PAC207";
    case PixelFormat::V4L2_M310:
      return "V4L2_PIX_FMT_MR97310A";
    case PixelFormat::V4L2_JL20:
      return "V4L2_PIX_FMT_JL2005BCD";
    case PixelFormat::V4L2_SONX:
      return "V4L2_PIX_FMT_SN9C2028";
    case PixelFormat::V4L2_905C:
      return "V4L2_PIX_FMT_SQ905C";
    case PixelFormat::V4L2_PJPG:
      return "V4L2_PIX_FMT_PJPG";
    case PixelFormat::V4L2_O511:
      return "V4L2_PIX_FMT_OV511";
    case PixelFormat::V4L2_O518:
      return "V4L2_PIX_FMT_OV518";
    case PixelFormat::V4L2_S680:
      return "V4L2_PIX_FMT_STV0680";
    case PixelFormat::V4L2_TM60:
      return "V4L2_PIX_FMT_TM6000";
    case PixelFormat::V4L2_CITV:
      return "V4L2_PIX_FMT_CIT_YYVYUY";
    case PixelFormat::V4L2_KONI:
      return "V4L2_PIX_FMT_KONICA420";
    case PixelFormat::V4L2_JPGL:
      return "V4L2_PIX_FMT_JPGL";
    case PixelFormat::V4L2_S401:
      return "V4L2_PIX_FMT_SE401";
    case PixelFormat::V4L2_S5CI:
      return "V4L2_PIX_FMT_S5C_UYVY_JPG";
    case PixelFormat::V4L2_Y8I:
      return "V4L2_PIX_FMT_Y8I";
    case PixelFormat::V4L2_Y12I:
      return "V4L2_PIX_FMT_Y12I";
    case PixelFormat::V4L2_Z16:
      return "V4L2_PIX_FMT_Z16";
    case PixelFormat::V4L2_MT21:
      return "V4L2_PIX_FMT_MT21C";
    case PixelFormat::V4L2_INZI:
      return "V4L2_PIX_FMT_INZI";
    case PixelFormat::V4L2_ST12:
      return "V4L2_PIX_FMT_SUNXI_TILED_NV12";
    case PixelFormat::V4L2_CNF4:
      return "V4L2_PIX_FMT_CNF4";
    case PixelFormat::V4L2_ip3b:
      return "V4L2_PIX_FMT_IPU3_SBGGR10";
    case PixelFormat::V4L2_ip3g:
      return "V4L2_PIX_FMT_IPU3_SGBRG10";
    case PixelFormat::V4L2_ip3G:
      return "V4L2_PIX_FMT_IPU3_SGRBG10";
    case PixelFormat::V4L2_ip3r:
      return "V4L2_PIX_FMT_IPU3_SRGGB10";
    default:
      return "UNKNOWN";
  }
}

inline constexpr std::optional<PixelFormat> fourcc2format(uint32_t fourcc) {
  switch (fourcc) {
    case V4L2_PIX_FMT_RGB332:
      return PixelFormat::V4L2_RGB1;
    case V4L2_PIX_FMT_RGB444:
      return PixelFormat::V4L2_R444;
    case V4L2_PIX_FMT_ARGB444:
      return PixelFormat::V4L2_AR12;
    case V4L2_PIX_FMT_XRGB444:
      return PixelFormat::V4L2_XR12;
    case V4L2_PIX_FMT_RGBA444:
      return PixelFormat::V4L2_RA12;
    case V4L2_PIX_FMT_RGBX444:
      return PixelFormat::V4L2_RX12;
    case V4L2_PIX_FMT_ABGR444:
      return PixelFormat::V4L2_AB12;
    case V4L2_PIX_FMT_XBGR444:
      return PixelFormat::V4L2_XB12;
    case V4L2_PIX_FMT_BGRA444:
      return PixelFormat::V4L2_GA12;
    case V4L2_PIX_FMT_BGRX444:
      return PixelFormat::V4L2_BX12;
    case V4L2_PIX_FMT_RGB555:
      return PixelFormat::V4L2_RGBO;
    case V4L2_PIX_FMT_ARGB555:
      return PixelFormat::V4L2_AR15;
    case V4L2_PIX_FMT_XRGB555:
      return PixelFormat::V4L2_XR15;
    case V4L2_PIX_FMT_RGBA555:
      return PixelFormat::V4L2_RA15;
    case V4L2_PIX_FMT_RGBX555:
      return PixelFormat::V4L2_RX15;
    case V4L2_PIX_FMT_ABGR555:
      return PixelFormat::V4L2_AB15;
    case V4L2_PIX_FMT_XBGR555:
      return PixelFormat::V4L2_XB15;
    case V4L2_PIX_FMT_BGRA555:
      return PixelFormat::V4L2_BA15;
    case V4L2_PIX_FMT_BGRX555:
      return PixelFormat::V4L2_BX15;
    case V4L2_PIX_FMT_RGB565:
      return PixelFormat::V4L2_RGBP;
    case V4L2_PIX_FMT_RGB555X:
      return PixelFormat::V4L2_RGBQ;
    case V4L2_PIX_FMT_RGB565X:
      return PixelFormat::V4L2_RGBR;
    case V4L2_PIX_FMT_BGR666:
      return PixelFormat::V4L2_BGRH;
    case V4L2_PIX_FMT_BGR24:
      return PixelFormat::V4L2_BGR3;
    case V4L2_PIX_FMT_RGB24:
      return PixelFormat::V4L2_RGB3;
    case V4L2_PIX_FMT_BGR32:
      return PixelFormat::V4L2_BGR4;
    case V4L2_PIX_FMT_ABGR32:
      return PixelFormat::V4L2_AR24;
    case V4L2_PIX_FMT_XBGR32:
      return PixelFormat::V4L2_XR24;
    case V4L2_PIX_FMT_BGRA32:
      return PixelFormat::V4L2_RA24;
    case V4L2_PIX_FMT_BGRX32:
      return PixelFormat::V4L2_RX24;
    case V4L2_PIX_FMT_RGB32:
      return PixelFormat::V4L2_RGB4;
    case V4L2_PIX_FMT_RGBA32:
      return PixelFormat::V4L2_AB24;
    case V4L2_PIX_FMT_RGBX32:
      return PixelFormat::V4L2_XB24;
    case V4L2_PIX_FMT_ARGB32:
      return PixelFormat::V4L2_BA24;
    case V4L2_PIX_FMT_XRGB32:
      return PixelFormat::V4L2_BX24;
    case V4L2_PIX_FMT_GREY:
      return PixelFormat::V4L2_GREY;
    case V4L2_PIX_FMT_Y4:
      return PixelFormat::V4L2_Y04;
    case V4L2_PIX_FMT_Y6:
      return PixelFormat::V4L2_Y06;
    case V4L2_PIX_FMT_Y10:
      return PixelFormat::V4L2_Y10;
    case V4L2_PIX_FMT_Y12:
      return PixelFormat::V4L2_Y12;
    case V4L2_PIX_FMT_Y16:
      return PixelFormat::V4L2_Y16;
    case V4L2_PIX_FMT_Y10BPACK:
      return PixelFormat::V4L2_Y10B;
    case V4L2_PIX_FMT_Y10P:
      return PixelFormat::V4L2_Y10P;
    case V4L2_PIX_FMT_PAL8:
      return PixelFormat::V4L2_PAL8;
    case V4L2_PIX_FMT_UV8:
      return PixelFormat::V4L2_UV8;
    case V4L2_PIX_FMT_YUYV:
      return PixelFormat::V4L2_YUYV;
    case V4L2_PIX_FMT_YYUV:
      return PixelFormat::V4L2_YYUV;
    case V4L2_PIX_FMT_YVYU:
      return PixelFormat::V4L2_YVYU;
    case V4L2_PIX_FMT_UYVY:
      return PixelFormat::V4L2_UYVY;
    case V4L2_PIX_FMT_VYUY:
      return PixelFormat::V4L2_VYUY;
    case V4L2_PIX_FMT_Y41P:
      return PixelFormat::V4L2_Y41P;
    case V4L2_PIX_FMT_YUV444:
      return PixelFormat::V4L2_Y444;
    case V4L2_PIX_FMT_YUV555:
      return PixelFormat::V4L2_YUVO;
    case V4L2_PIX_FMT_YUV565:
      return PixelFormat::V4L2_YUVP;
    case V4L2_PIX_FMT_YUV32:
      return PixelFormat::V4L2_YUV4;
    case V4L2_PIX_FMT_AYUV32:
      return PixelFormat::V4L2_AYUV;
    case V4L2_PIX_FMT_XYUV32:
      return PixelFormat::V4L2_XYUV;
    case V4L2_PIX_FMT_VUYA32:
      return PixelFormat::V4L2_VUYA;
    case V4L2_PIX_FMT_VUYX32:
      return PixelFormat::V4L2_VUYX;
    case V4L2_PIX_FMT_HI240:
      return PixelFormat::V4L2_HI24;
    case V4L2_PIX_FMT_HM12:
      return PixelFormat::V4L2_HM12;
    case V4L2_PIX_FMT_M420:
      return PixelFormat::V4L2_M420;
    case V4L2_PIX_FMT_NV12:
      return PixelFormat::V4L2_NV12;
    case V4L2_PIX_FMT_NV21:
      return PixelFormat::V4L2_NV21;
    case V4L2_PIX_FMT_NV16:
      return PixelFormat::V4L2_NV16;
    case V4L2_PIX_FMT_NV61:
      return PixelFormat::V4L2_NV61;
    case V4L2_PIX_FMT_NV24:
      return PixelFormat::V4L2_NV24;
    case V4L2_PIX_FMT_NV42:
      return PixelFormat::V4L2_NV42;
    case V4L2_PIX_FMT_NV12M:
      return PixelFormat::V4L2_NM12;
    case V4L2_PIX_FMT_NV21M:
      return PixelFormat::V4L2_NM21;
    case V4L2_PIX_FMT_NV16M:
      return PixelFormat::V4L2_NM16;
    case V4L2_PIX_FMT_NV61M:
      return PixelFormat::V4L2_NM61;
    case V4L2_PIX_FMT_NV12MT:
      return PixelFormat::V4L2_TM12;
    case V4L2_PIX_FMT_NV12MT_16X16:
      return PixelFormat::V4L2_VM12;
    case V4L2_PIX_FMT_YUV410:
      return PixelFormat::V4L2_YUV9;
    case V4L2_PIX_FMT_YVU410:
      return PixelFormat::V4L2_YVU9;
    case V4L2_PIX_FMT_YUV411P:
      return PixelFormat::V4L2_411P;
    case V4L2_PIX_FMT_YUV420:
      return PixelFormat::V4L2_YU12;
    case V4L2_PIX_FMT_YVU420:
      return PixelFormat::V4L2_YV12;
    case V4L2_PIX_FMT_YUV422P:
      return PixelFormat::V4L2_422P;
    case V4L2_PIX_FMT_YUV420M:
      return PixelFormat::V4L2_YM12;
    case V4L2_PIX_FMT_YVU420M:
      return PixelFormat::V4L2_YM21;
    case V4L2_PIX_FMT_YUV422M:
      return PixelFormat::V4L2_YM16;
    case V4L2_PIX_FMT_YVU422M:
      return PixelFormat::V4L2_YM61;
    case V4L2_PIX_FMT_YUV444M:
      return PixelFormat::V4L2_YM24;
    case V4L2_PIX_FMT_YVU444M:
      return PixelFormat::V4L2_YM42;
    case V4L2_PIX_FMT_SBGGR8:
      return PixelFormat::V4L2_BA81;
    case V4L2_PIX_FMT_SGBRG8:
      return PixelFormat::V4L2_GBRG;
    case V4L2_PIX_FMT_SGRBG8:
      return PixelFormat::V4L2_GRBG;
    case V4L2_PIX_FMT_SRGGB8:
      return PixelFormat::V4L2_RGGB;
    case V4L2_PIX_FMT_SBGGR10:
      return PixelFormat::V4L2_BG10;
    case V4L2_PIX_FMT_SGBRG10:
      return PixelFormat::V4L2_GB10;
    case V4L2_PIX_FMT_SGRBG10:
      return PixelFormat::V4L2_BA10;
    case V4L2_PIX_FMT_SRGGB10:
      return PixelFormat::V4L2_RG10;
    case V4L2_PIX_FMT_SBGGR10P:
      return PixelFormat::V4L2_pBAA;
    case V4L2_PIX_FMT_SGBRG10P:
      return PixelFormat::V4L2_pGAA;
    case V4L2_PIX_FMT_SGRBG10P:
      return PixelFormat::V4L2_pgAA;
    case V4L2_PIX_FMT_SRGGB10P:
      return PixelFormat::V4L2_pRAA;
    case V4L2_PIX_FMT_SBGGR10ALAW8:
      return PixelFormat::V4L2_aBA8;
    case V4L2_PIX_FMT_SGBRG10ALAW8:
      return PixelFormat::V4L2_aGA8;
    case V4L2_PIX_FMT_SGRBG10ALAW8:
      return PixelFormat::V4L2_agA8;
    case V4L2_PIX_FMT_SRGGB10ALAW8:
      return PixelFormat::V4L2_aRA8;
    case V4L2_PIX_FMT_SBGGR10DPCM8:
      return PixelFormat::V4L2_bBA8;
    case V4L2_PIX_FMT_SGBRG10DPCM8:
      return PixelFormat::V4L2_bGA8;
    case V4L2_PIX_FMT_SGRBG10DPCM8:
      return PixelFormat::V4L2_BD10;
    case V4L2_PIX_FMT_SRGGB10DPCM8:
      return PixelFormat::V4L2_bRA8;
    case V4L2_PIX_FMT_SBGGR12:
      return PixelFormat::V4L2_BG12;
    case V4L2_PIX_FMT_SGBRG12:
      return PixelFormat::V4L2_GB12;
    case V4L2_PIX_FMT_SGRBG12:
      return PixelFormat::V4L2_BA12;
    case V4L2_PIX_FMT_SRGGB12:
      return PixelFormat::V4L2_RG12;
    case V4L2_PIX_FMT_SBGGR12P:
      return PixelFormat::V4L2_pBCC;
    case V4L2_PIX_FMT_SGBRG12P:
      return PixelFormat::V4L2_pGCC;
    case V4L2_PIX_FMT_SGRBG12P:
      return PixelFormat::V4L2_pgCC;
    case V4L2_PIX_FMT_SRGGB12P:
      return PixelFormat::V4L2_pRCC;
    case V4L2_PIX_FMT_SBGGR14P:
      return PixelFormat::V4L2_pBEE;
    case V4L2_PIX_FMT_SGBRG14P:
      return PixelFormat::V4L2_pGEE;
    case V4L2_PIX_FMT_SGRBG14P:
      return PixelFormat::V4L2_pgEE;
    case V4L2_PIX_FMT_SRGGB14P:
      return PixelFormat::V4L2_pREE;
    case V4L2_PIX_FMT_SBGGR16:
      return PixelFormat::V4L2_BYR2;
    case V4L2_PIX_FMT_SGBRG16:
      return PixelFormat::V4L2_GB16;
    case V4L2_PIX_FMT_SGRBG16:
      return PixelFormat::V4L2_GR16;
    case V4L2_PIX_FMT_SRGGB16:
      return PixelFormat::V4L2_RG16;
    case V4L2_PIX_FMT_HSV24:
      return PixelFormat::V4L2_HSV3;
    case V4L2_PIX_FMT_HSV32:
      return PixelFormat::V4L2_HSV4;
    case V4L2_PIX_FMT_MJPEG:
      return PixelFormat::V4L2_MJPG;
    case V4L2_PIX_FMT_JPEG:
      return PixelFormat::V4L2_JPEG;
    case V4L2_PIX_FMT_DV:
      return PixelFormat::V4L2_dvsd;
    case V4L2_PIX_FMT_MPEG:
      return PixelFormat::V4L2_MPEG;
    case V4L2_PIX_FMT_H264:
      return PixelFormat::V4L2_H264;
    case V4L2_PIX_FMT_H264_NO_SC:
      return PixelFormat::V4L2_AVC1;
    case V4L2_PIX_FMT_H264_MVC:
      return PixelFormat::V4L2_M264;
    case V4L2_PIX_FMT_H263:
      return PixelFormat::V4L2_H263;
    case V4L2_PIX_FMT_MPEG1:
      return PixelFormat::V4L2_MPG1;
    case V4L2_PIX_FMT_MPEG2:
      return PixelFormat::V4L2_MPG2;
    case V4L2_PIX_FMT_MPEG2_SLICE:
      return PixelFormat::V4L2_MG2S;
    case V4L2_PIX_FMT_MPEG4:
      return PixelFormat::V4L2_MPG4;
    case V4L2_PIX_FMT_XVID:
      return PixelFormat::V4L2_XVID;
    case V4L2_PIX_FMT_VC1_ANNEX_G:
      return PixelFormat::V4L2_VC1G;
    case V4L2_PIX_FMT_VC1_ANNEX_L:
      return PixelFormat::V4L2_VC1L;
    case V4L2_PIX_FMT_VP8:
      return PixelFormat::V4L2_VP80;
    case V4L2_PIX_FMT_VP9:
      return PixelFormat::V4L2_VP90;
    case V4L2_PIX_FMT_HEVC:
      return PixelFormat::V4L2_HEVC;
    case V4L2_PIX_FMT_FWHT:
      return PixelFormat::V4L2_FWHT;
    case V4L2_PIX_FMT_FWHT_STATELESS:
      return PixelFormat::V4L2_SFWH;
    case V4L2_PIX_FMT_CPIA1:
      return PixelFormat::V4L2_CPIA;
    case V4L2_PIX_FMT_WNVA:
      return PixelFormat::V4L2_WNVA;
    case V4L2_PIX_FMT_SN9C10X:
      return PixelFormat::V4L2_S910;
    case V4L2_PIX_FMT_SN9C20X_I420:
      return PixelFormat::V4L2_S920;
    case V4L2_PIX_FMT_PWC1:
      return PixelFormat::V4L2_PWC1;
    case V4L2_PIX_FMT_PWC2:
      return PixelFormat::V4L2_PWC2;
    case V4L2_PIX_FMT_ET61X251:
      return PixelFormat::V4L2_E625;
    case V4L2_PIX_FMT_SPCA501:
      return PixelFormat::V4L2_S501;
    case V4L2_PIX_FMT_SPCA505:
      return PixelFormat::V4L2_S505;
    case V4L2_PIX_FMT_SPCA508:
      return PixelFormat::V4L2_S508;
    case V4L2_PIX_FMT_SPCA561:
      return PixelFormat::V4L2_S561;
    case V4L2_PIX_FMT_PAC207:
      return PixelFormat::V4L2_P207;
    case V4L2_PIX_FMT_MR97310A:
      return PixelFormat::V4L2_M310;
    case V4L2_PIX_FMT_JL2005BCD:
      return PixelFormat::V4L2_JL20;
    case V4L2_PIX_FMT_SN9C2028:
      return PixelFormat::V4L2_SONX;
    case V4L2_PIX_FMT_SQ905C:
      return PixelFormat::V4L2_905C;
    case V4L2_PIX_FMT_PJPG:
      return PixelFormat::V4L2_PJPG;
    case V4L2_PIX_FMT_OV511:
      return PixelFormat::V4L2_O511;
    case V4L2_PIX_FMT_OV518:
      return PixelFormat::V4L2_O518;
    case V4L2_PIX_FMT_STV0680:
      return PixelFormat::V4L2_S680;
    case V4L2_PIX_FMT_TM6000:
      return PixelFormat::V4L2_TM60;
    case V4L2_PIX_FMT_CIT_YYVYUY:
      return PixelFormat::V4L2_CITV;
    case V4L2_PIX_FMT_KONICA420:
      return PixelFormat::V4L2_KONI;
    case V4L2_PIX_FMT_JPGL:
      return PixelFormat::V4L2_JPGL;
    case V4L2_PIX_FMT_SE401:
      return PixelFormat::V4L2_S401;
    case V4L2_PIX_FMT_S5C_UYVY_JPG:
      return PixelFormat::V4L2_S5CI;
    case V4L2_PIX_FMT_Y8I:
      return PixelFormat::V4L2_Y8I;
    case V4L2_PIX_FMT_Y12I:
      return PixelFormat::V4L2_Y12I;
    case V4L2_PIX_FMT_Z16:
      return PixelFormat::V4L2_Z16;
    case V4L2_PIX_FMT_MT21C:
      return PixelFormat::V4L2_MT21;
    case V4L2_PIX_FMT_INZI:
      return PixelFormat::V4L2_INZI;
    case V4L2_PIX_FMT_SUNXI_TILED_NV12:
      return PixelFormat::V4L2_ST12;
    case V4L2_PIX_FMT_CNF4:
      return PixelFormat::V4L2_CNF4;
    case V4L2_PIX_FMT_IPU3_SBGGR10:
      return PixelFormat::V4L2_ip3b;
    case V4L2_PIX_FMT_IPU3_SGBRG10:
      return PixelFormat::V4L2_ip3g;
    case V4L2_PIX_FMT_IPU3_SGRBG10:
      return PixelFormat::V4L2_ip3G;
    case V4L2_PIX_FMT_IPU3_SRGGB10:
      return PixelFormat::V4L2_ip3r;
    default:
      return std::nullopt;
  }
}

}  //  namespace lirs::formats
