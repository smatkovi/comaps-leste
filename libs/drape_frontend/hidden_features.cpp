#include "drape_frontend/hidden_features.hpp"

#include "indexer/classificator.hpp"
#include "indexer/feature_data.hpp"
#include "indexer/ftypes_matcher.hpp"

#include <initializer_list>

namespace df
{
namespace
{
class HighwayChecker : public ftypes::BaseChecker
{
public:
  explicit HighwayChecker(std::initializer_list<char const *> kinds) : BaseChecker(2 /* level */)
  {
    for (auto const * kind : kinds)
      m_types.push_back(classif().GetTypeByPath({"highway", kind}));
  }
};
}  // namespace

bool IsHiddenWay(uint32_t hiddenFeatures, feature::TypesHolder const & types)
{
  static HighwayChecker const footways({"footway", "path", "pedestrian", "cycleway", "bridleway", "steps", "ladder",
                                        "track"});
  static HighwayChecker const roads({"motorway", "motorway_link", "trunk", "trunk_link", "primary", "primary_link",
                                     "secondary", "secondary_link", "tertiary", "tertiary_link", "unclassified",
                                     "residential", "living_street", "service", "road", "busway", "construction"});
  return ((hiddenFeatures & HiddenFootways) != 0 && footways(types)) ||
         ((hiddenFeatures & HiddenRoads) != 0 && roads(types));
}
}  // namespace df
