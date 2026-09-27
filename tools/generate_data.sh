#!/usr/bin/env bash
# Generates the runtime data (strings, categories, drawing rules, symbol skins)
# into a copy of data/ in the build tree.
#
# Usage: generate_data.sh <source root> <output data dir> <skin_generator_tool> <protoc>
set -euo pipefail

SRC="$1"
OUT="$2"
SKIN_GENERATOR="$3"
PROTOC="$4"
WORK="$OUT.work"

rm -rf "$OUT" "$WORK"
mkdir -p "$OUT" "$WORK"
cp -a "$SRC/data/." "$OUT/"

echo "Generating json strings..."
python3 "$SRC/tools/python/generate_json_strings.py" "$OUT"

echo "Generating search categories..."
python3 "$SRC/tools/python/categories/json_to_txt.py" "$OUT/categories-strings" "$OUT/categories.txt"

echo "Generating drawing rules..."
"$PROTOC" --proto_path="$SRC/libs/indexer" --python_out="$WORK" "$SRC/libs/indexer/drules_struct.proto"

BuildDrawingRules() {
  local styleType=$1 styleName=$2 suffix=$3
  echo "Building drawing rules for style $styleType/$styleName"
  PYTHONPATH="$WORK" python3 "$SRC/tools/kothic/libkomwm.py" \
    -s "$OUT/styles/$styleType/$styleName/style.mapcss" \
    -o "$OUT/drules_proto$suffix" \
    -p "$OUT/styles/$styleType/include/"
}

BuildDrawingRules default  light _default_light
BuildDrawingRules default  dark _default_dark
BuildDrawingRules outdoors  light _outdoors_light
BuildDrawingRules outdoors  dark _outdoors_dark
BuildDrawingRules walking  light _walking_light
BuildDrawingRules walking_outdoor  light _walking_outdoor_light
BuildDrawingRules walking  dark _walking_dark
BuildDrawingRules walking_outdoor  dark _walking_outdoor_dark
BuildDrawingRules cycling  light _cycling_light
BuildDrawingRules cycling_outdoor  light _cycling_outdoor_light
BuildDrawingRules cycling  dark _cycling_dark
BuildDrawingRules cycling_outdoor  dark _cycling_outdoor_dark
BuildDrawingRules driving  light _driving_light
BuildDrawingRules driving_outdoor  light _driving_outdoor_light
BuildDrawingRules driving  dark _driving_dark
BuildDrawingRules driving_outdoor  dark _driving_outdoor_dark
BuildDrawingRules public-transport  light _public-transport_light
BuildDrawingRules public-transport_outdoor  light _public-transport_outdoor_light
BuildDrawingRules public-transport  dark _public-transport_dark
BuildDrawingRules public-transport_outdoor  dark _public-transport_outdoor_dark
# Keep vehicle style last to produce same visibility.txt & classificator.txt
BuildDrawingRules vehicle  light _vehicle_light
BuildDrawingRules vehicle  dark _vehicle_dark

echo "Generating symbols..."
export QT_QPA_PLATFORM=offscreen

BuildSkin() {
  local styleName=$1 resourceName=$2 symbolSize=$3
  local stylePath="$OUT/styles/default/$styleName"
  echo "Building skin for $styleName/$resourceName"
  rm -rf "$stylePath/symbols/png"
  ln -s "$stylePath/$resourceName" "$stylePath/symbols/png"
  mkdir -p "$OUT/symbols/$resourceName/$styleName"
  "$SKIN_GENERATOR" --symbolWidth "$symbolSize" --symbolHeight "$symbolSize" \
    --symbolsDir "$stylePath/symbols" --skinName "$OUT/symbols/$resourceName/$styleName/basic"
  rm -f "$stylePath/symbols/png"
}

for style in dark light; do
  BuildSkin $style mdpi    18
  BuildSkin $style hdpi    27
  BuildSkin $style xhdpi   36
  BuildSkin $style 6plus   43
  BuildSkin $style xxhdpi  54
  BuildSkin $style xxxhdpi 64
done

rm -rf "$WORK"
