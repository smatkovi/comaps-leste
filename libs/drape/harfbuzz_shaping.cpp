#include "drape/harfbuzz_shaping.hpp"

#include "base/assert.hpp"
#include "base/string_utils.hpp"

#include <algorithm>
#include <sstream>
#include <string>

#include <utf8/unchecked.h>

namespace harfbuzz_shaping
{
namespace
{
void AddSegment(TextSegments & segments, int32_t start, int32_t end, hb_script_t script)
{
  // Without a bidi algorithm (built without ICU) the direction of a run follows its script.
  hb_direction_t direction = hb_script_get_horizontal_direction(script);
  if (direction == HB_DIRECTION_INVALID)
    direction = HB_DIRECTION_LTR;
  segments.m_segments.emplace_back(start, end - start, script, direction);
}

// Splits the text into runs of a common script. Common and Inherited characters (spaces, digits,
// punctuation, combining marks) join the run they are in.
void GetSingleTextLineRuns(TextSegments & segments)
{
  auto const & text = segments.m_text;
  hb_unicode_funcs_t * const unicodeFuncs = hb_unicode_funcs_get_default();

  int32_t runStart = 0;
  hb_script_t runScript = HB_SCRIPT_COMMON;
  for (auto it = text.begin(); it != text.end();)
  {
    auto const charStart = static_cast<int32_t>(it - text.begin());
    hb_script_t const script = hb_unicode_script(unicodeFuncs, utf8::unchecked::next16(it));
    if (script == HB_SCRIPT_COMMON || script == HB_SCRIPT_INHERITED || script == HB_SCRIPT_UNKNOWN ||
        script == runScript)
      continue;

    if (runScript != HB_SCRIPT_COMMON)
    {
      AddSegment(segments, runStart, charStart, runScript);
      runStart = charStart;
    }
    runScript = script;
  }
  AddSegment(segments, runStart, static_cast<int32_t>(text.length()), runScript);
}

void ReorderRTL(TextSegments & segments)
{
  // TODO(AB): Optimize implementation to use indexes to segments instead of copying them.
  auto it = segments.m_segments.begin();
  auto const end = segments.m_segments.end();
  // TODO(AB): Line (default rendering) direction is determined by the first segment. It should be defined as
  // a parameter depending on the language.
  auto const lineDirection = it->m_direction;
  while (it != end)
  {
    if (it->m_direction == lineDirection)
      ++it;
    else
    {
      auto const start = it++;
      while (it != end && it->m_direction != lineDirection)
        ++it;
      std::reverse(start, it);
    }
  }
  if (lineDirection != HB_DIRECTION_LTR)
    std::reverse(segments.m_segments.begin(), end);
}
}  // namespace

TextSegments GetTextSegments(std::string_view utf8)
{
  ASSERT(!utf8.empty(), ("Shaping of empty strings is not supported"));
  ASSERT(std::string::npos == utf8.find_first_of("\n"), ("Shaping with line breaks is not supported", utf8));

  // TODO(AB): Can unnecessary conversion/allocation be avoided?
  TextSegments segments{strings::ToUtf16(utf8), {}};
  // TODO(AB): Runs are not split by breaking chars and by different fonts.
  GetSingleTextLineRuns(segments);
  ReorderRTL(segments);
  return segments;
}

std::string DebugPrint(TextSegment const & segment)
{
  std::stringstream ss;
  ss << "TextSegment[start=" << segment.m_start << ", length=" << segment.m_length << ", script=" << segment.m_script
     << ", direction=" << segment.m_direction << ']';
  return ss.str();
}

}  // namespace harfbuzz_shaping
