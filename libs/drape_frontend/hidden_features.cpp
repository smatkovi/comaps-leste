#include "drape_frontend/hidden_features.hpp"

#include "indexer/classificator.hpp"
#include "indexer/feature_data.hpp"
#include "indexer/ftypes_matcher.hpp"

#include <initializer_list>

namespace df
{
namespace
{
// Every type under a root of the classificator ("shop", "barrier", ...).
class RootChecker : public ftypes::BaseChecker
{
public:
  explicit RootChecker(std::initializer_list<char const *> roots) : BaseChecker(1 /* level */)
  {
    for (auto const * root : roots)
      m_types.push_back(classif().GetTypeByPath({root}));
  }
};

// The listed types under one root ("highway|footway", "amenity|waste_basket", ...).
class KindChecker : public ftypes::BaseChecker
{
public:
  KindChecker(char const * root, std::initializer_list<char const *> kinds) : BaseChecker(2 /* level */)
  {
    for (auto const * kind : kinds)
      m_types.push_back(classif().GetTypeByPath({root, kind}));
  }
};

// Gyms, sports centres, escape rooms and other indoor venues; parks, pitches and other open spaces stay.
class LeisureVenueChecker : public ftypes::BaseChecker
{
public:
  LeisureVenueChecker() : BaseChecker(2 /* level */)
  {
    for (auto const * venue :
         {"sports_centre", "fitness_centre", "sports_hall", "escape_game", "bowling_alley", "sauna", "dance",
          "indoor_play", "adult_gaming_centre", "amusement_arcade", "hackerspace", "ice_rink", "miniature_golf"})
      m_types.push_back(classif().GetTypeByPath({"leisure", venue}));
  }
};

// Points that only clutter the map at the zooms they appear at and say nothing a phone map is used
// for: single trees and the survey markers of a land survey. They have no filter of their own.
bool IsNeverDrawnPoi(feature::TypesHolder const & types)
{
  static KindChecker const trees("natural", {"tree"});
  static KindChecker const surveyPoints("man_made", {"survey_point"});
  return trees(types) || surveyPoints(types);
}
}  // namespace

bool IsHiddenWay(uint32_t hiddenFeatures, feature::TypesHolder const & types)
{
  static KindChecker const footways("highway", {"footway", "path", "pedestrian", "cycleway", "bridleway", "steps",
                                                "ladder", "track"});
  static KindChecker const roads("highway", {"motorway", "motorway_link", "trunk", "trunk_link", "primary",
                                             "primary_link", "secondary", "secondary_link", "tertiary", "tertiary_link",
                                             "unclassified", "residential", "living_street", "service", "road",
                                             "busway", "construction"});
  // Taxiways belong to neither mode: only shown with both roads and footpaths.
  static KindChecker const taxiways("aeroway", {"taxiway"});
  bool const hideFootways = (hiddenFeatures & HiddenFootways) != 0;
  bool const hideRoads = (hiddenFeatures & HiddenRoads) != 0;
  return (hideFootways && footways(types)) || (hideRoads && roads(types)) ||
         ((hideFootways || hideRoads) && taxiways(types));
}

bool IsHiddenPoi(uint32_t hiddenFeatures, feature::TypesHolder const & types)
{
  static RootChecker const shops({"shop"});
  static RootChecker const tourism({"tourism"});
  static RootChecker const services({"amenity", "office", "craft", "healthcare", "sport"});
  static LeisureVenueChecker const leisureVenues;
  static RootChecker const entrances({"entrance"});
  static RootChecker const barriers({"barrier"});
  static KindChecker const waste("amenity", {"waste_basket", "waste_disposal"});
  static KindChecker const elevators("highway", {"elevator"});

  if (IsNeverDrawnPoi(types))
    return true;

  auto const hidden = [hiddenFeatures](HiddenFeatures group) { return (hiddenFeatures & group) != 0; };
  bool const eat = ftypes::IsEatChecker::Instance()(types);
  // Bins are amenities, but have their own filter: 'Other places' leaves them to it.
  return (hidden(HiddenFood) && eat) || (hidden(HiddenShops) && shops(types)) ||
         (hidden(HiddenTourism) && tourism(types)) ||
         (hidden(HiddenServices) && !eat && !waste(types) && (services(types) || leisureVenues(types))) ||
         (hidden(HiddenEntrances) && entrances(types)) || (hidden(HiddenBarriers) && barriers(types)) ||
         (hidden(HiddenWaste) && waste(types)) || (hidden(HiddenElevators) && elevators(types));
}
}  // namespace df
