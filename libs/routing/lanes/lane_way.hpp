#pragma once

#include "base/assert.hpp"

#include <bitset>
#include <initializer_list>
#include <string>

namespace routing::turns::lanes
{
enum class LaneWay : std::uint8_t
{
  None = 0,
  ReverseLeft,
  SharpLeft,
  Left,
  MergeToLeft,
  SlightLeft,
  Through,
  SlightRight,
  MergeToRight,
  Right,
  SharpRight,
  ReverseRight,

  Count
};

// std::bitset is constexpr from C++23 in libstdc++ 13 and libc++ 16 on; Debian 12's gcc 12 has a
// bitset that is not, so there the members below are not constexpr either.
#if defined(__cpp_lib_constexpr_bitset)
#define LANE_WAYS_CONSTEXPR constexpr
#else
#define LANE_WAYS_CONSTEXPR
#endif

class LaneWays
{
  using LaneWaysT = std::bitset<static_cast<std::uint8_t>(LaneWay::Count)>;

  friend std::string DebugPrint(LaneWays const & laneWays);

public:
  constexpr LaneWays() = default;
  LANE_WAYS_CONSTEXPR LaneWays(std::initializer_list<LaneWay> const laneWays)
  {
    for (auto const & laneWay : laneWays)
      Add(laneWay);
  }

  LANE_WAYS_CONSTEXPR bool operator==(LaneWays const & rhs) const { return m_laneWays == rhs.m_laneWays; }

  LANE_WAYS_CONSTEXPR void Add(LaneWay laneWay)
  {
    ASSERT_LESS(laneWay, LaneWay::Count, ());
    m_laneWays.set(static_cast<std::uint8_t>(laneWay));
  }

  LANE_WAYS_CONSTEXPR void Remove(LaneWay laneWay)
  {
    ASSERT_LESS(laneWay, LaneWay::Count, ());
    m_laneWays.reset(static_cast<std::uint8_t>(laneWay));
  }

  LANE_WAYS_CONSTEXPR bool Contains(LaneWay laneWay) const
  {
    ASSERT_LESS(laneWay, LaneWay::Count, ());
    return m_laneWays.test(static_cast<std::uint8_t>(laneWay));
  }

  /// An unrestricted lane is a lane that has no restrictions, i.e., it contains no lane ways.
  LANE_WAYS_CONSTEXPR bool IsUnrestricted() const
  {
    return m_laneWays.none() || (m_laneWays.count() == 1 && Contains(LaneWay::None));
  }

  [[nodiscard]] std::vector<LaneWay> GetActiveLaneWays() const
  {
    std::vector<LaneWay> result;
    for (std::size_t i = 0; i < m_laneWays.size(); ++i)
      if (m_laneWays.test(i))
        result.emplace_back(static_cast<LaneWay>(i));
    return result;
  }

private:
  LaneWaysT m_laneWays;
};

std::string DebugPrint(LaneWay laneWay);
std::string DebugPrint(LaneWays const & laneWays);
}  // namespace routing::turns::lanes
