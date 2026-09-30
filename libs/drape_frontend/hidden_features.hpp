#pragma once

#include <cstdint>

namespace feature
{
class TypesHolder;
}  // namespace feature

namespace df
{
enum HiddenFeatures : uint32_t
{
  HiddenFood = 1u << 0,
  HiddenShops = 1u << 1,
  HiddenTourism = 1u << 2,
  HiddenServices = 1u << 3,
  HiddenHouseNumbers = 1u << 4,
  HiddenBuildings = 1u << 5,
  HiddenFootways = 1u << 6,  // footways, paths, cycleways, tracks, steps
  HiddenRoads = 1u << 7,     // every road a car can use
  HiddenEntrances = 1u << 8,
  HiddenBarriers = 1u << 9,  // gates, bollards, stiles and the other barrier nodes
  HiddenWaste = 1u << 10,    // dumpsters and waste baskets
  HiddenElevators = 1u << 11,
};

/// Whether a way (line) with these types is of a kind the hidden mask removes (roads or footpaths).
bool IsHiddenWay(uint32_t hiddenFeatures, feature::TypesHolder const & types);

/// Whether a feature with these types gets no icon and no caption: either its group is in the hidden
/// mask, or it is of a kind that is never drawn (individual trees, survey points).
bool IsHiddenPoi(uint32_t hiddenFeatures, feature::TypesHolder const & types);
}  // namespace df
