#pragma once

#include <cstdint>

namespace df
{
enum HiddenFeatures : uint32_t
{
  HiddenFood = 1u << 0,
  HiddenShops = 1u << 1,
  HiddenTourism = 1u << 2,
  HiddenServices = 1u << 3,
  HiddenHouseNumbers = 1u << 4,
};
}  // namespace df
