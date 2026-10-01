#include "drape_frontend/hidden_features.hpp"

#include "indexer/classificator.hpp"
#include "indexer/feature_data.hpp"
#include "indexer/ftypes_matcher.hpp"

#include <initializer_list>

namespace df
{
namespace
{
class WayChecker : public ftypes::BaseChecker
{
public:
  WayChecker(char const * root, std::initializer_list<char const *> kinds) : BaseChecker(2 /* level */)
  {
    for (auto const * kind : kinds)
      m_types.push_back(classif().GetTypeByPath({root, kind}));
  }
};
}  // namespace

bool IsHiddenWay(uint32_t hiddenFeatures, feature::TypesHolder const & types)
{
  static WayChecker const footways("highway", {"footway", "path", "pedestrian", "cycleway", "bridleway", "steps",
                                               "ladder", "track"});
  static WayChecker const roads("highway", {"motorway", "motorway_link", "trunk", "trunk_link", "primary",
                                            "primary_link", "secondary", "secondary_link", "tertiary", "tertiary_link",
                                            "unclassified", "residential", "living_street", "service", "road", "busway",
                                            "construction"});
  // Taxiways belong to neither mode: only shown with both roads and footpaths.
  static WayChecker const taxiways("aeroway", {"taxiway"});
  bool const hideFootways = (hiddenFeatures & HiddenFootways) != 0;
  bool const hideRoads = (hiddenFeatures & HiddenRoads) != 0;
  return (hideFootways && footways(types)) || (hideRoads && roads(types)) ||
         ((hideFootways || hideRoads) && taxiways(types));
}
}  // namespace df
