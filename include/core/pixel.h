#ifndef SERVICE_TEMPLATE_SRC_CORE_PIXEL_H_
#define SERVICE_TEMPLATE_SRC_CORE_PIXEL_H_

#include <stdint.h>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <tuple>

#include "bytes.h"
#include "color_space.h"
#include "pnm_type.h"

namespace server::core::pnm::color_space {
using channel = double;

template <ColorSpace>
struct Pixel;

template <ColorSpace color>
bool operator==(const Pixel<color>& p1, const Pixel<color>& p2);

template <ColorSpace color>
Pixel<color> ConstructPixel(int32_t a, int32_t b, int32_t c) {
  return {a, b, c};
}

template <ColorSpace color>
std::tuple<int32_t, int32_t, int32_t> GetRaw(const Pixel<color>& pixel) {
  return {pixel.template get<0>(), pixel.template get<1>(),
          pixel.template get<2>()};
}

template <>
struct Pixel<ColorSpace::NONE> {
  Pixel() = default;
  byte scale{};
};

template <>
struct Pixel<ColorSpace::RGB> {
  Pixel() = default;
  Pixel(int32_t r, int32_t g, int32_t b)
      : red(r / 255.0), green(g / 255.0), blue(b / 255.0) {}
  channel red{};
  channel green{};
  channel blue{};
  template <std::size_t I>
  [[nodiscard]] int32_t get() const {
    return std::round(255 * std::get<I>(std::tie(red, green, blue)));
  }

  static Pixel white;
};

template <>
struct Pixel<ColorSpace::HSL> {
  Pixel() = default;
  Pixel(int32_t h, int32_t s, int32_t l)
      : hue(h / 255.0 * 360.0), saturation(s / 255.0), lightness(l / 255.0) {}
  channel hue{};
  channel saturation{};
  channel lightness{};
  template <std::size_t I>
  [[nodiscard]] int32_t get() const {
    if constexpr (I == 0)
      return std::round(hue * 255. / 360.);
    else
      return std::round(255 * std::get<I - 1>(std::tie(saturation, lightness)));
  }
};

template <>
struct Pixel<ColorSpace::HSV> {
  channel hue{};
  channel saturation{};
  channel value{};
};

template <ColorSpace From, ColorSpace To, Mask Channel = Mask::ALL>
struct ColorSpaceConversion {
  Pixel<To> operator()(const Pixel<From>& pixel);
};

template <>
struct ColorSpaceConversion<ColorSpace::RGB, ColorSpace::HSL> {
  Pixel<ColorSpace::HSL> operator()(const Pixel<ColorSpace::RGB>& pixel);
};

template <>
struct ColorSpaceConversion<ColorSpace::HSL, ColorSpace::RGB> {
  Pixel<ColorSpace::RGB> operator()(const Pixel<ColorSpace::HSL>& pixel);
};

template <>
struct ColorSpaceConversion<ColorSpace::RGB, ColorSpace::HSV> {
  Pixel<ColorSpace::HSV> operator()(const Pixel<ColorSpace::RGB>& pixel);
};

template <>
struct ColorSpaceConversion<ColorSpace::HSV, ColorSpace::RGB> {
  Pixel<ColorSpace::RGB> operator()(const Pixel<ColorSpace::HSV>& pixel);
};

template <ColorSpace From, Mask Channel>
struct ColorSpaceConversion<From, ColorSpace::NONE, Channel> {
  Pixel<ColorSpace::NONE> operator()(const Pixel<From>& pixel) {
    return {static_cast<byte>(
        pixel.template get<static_cast<std::size_t>(Channel)>())};
  }
};

template <ColorSpace Space>
Pixel<Space> AlphaBlending(const Pixel<Space>& background,
                           const Pixel<Space>& foreground, double alpha) {
  return ColorSpaceConversion<ColorSpace::RGB, Space>(AlphaBlending(
      ColorSpaceConversion<Space, ColorSpace::RGB>(background),
      ColorSpaceConversion<Space, ColorSpace::RGB>(foreground), alpha));
}

template <>
Pixel<ColorSpace::NONE> AlphaBlending(const Pixel<ColorSpace::NONE>& background,
                                      const Pixel<ColorSpace::NONE>& foreground,
                                      double alpha);

template <>
Pixel<ColorSpace::RGB> AlphaBlending(const Pixel<ColorSpace::RGB>& background,
                                     const Pixel<ColorSpace::RGB>& foreground,
                                     double alpha);

Pixel<ColorSpace::RGB> operator*(double lhs, Pixel<ColorSpace::RGB> pixel);

Pixel<ColorSpace::RGB> operator+(Pixel<ColorSpace::RGB> lhs,
                                 const Pixel<ColorSpace::RGB>& rhs);

Pixel<ColorSpace::RGB> operator+(Pixel<ColorSpace::RGB> lhs, double rhs);

}  // namespace server::core::pnm::color_space

#endif  // SERVICE_TEMPLATE_SRC_CORE_PIXEL_H_
