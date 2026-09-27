#pragma once

#include "i18n/localisation.hpp"

#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>

// Built without ICU, so there is no transliteration: every call reports
// "not transliterated" and names are shown and searched as they are.
class Transliteration
{
public:
  enum class Mode
  {
    Enabled,
    Disabled
  };

  static Transliteration & Instance();

  void Init(std::string const & icuDataDir);

  void SetMode(Mode mode);
  // Transliterates |sv| with transliterators set for |langCode| in StringUtf8Multilang
  // if mode is set to Enabled.
  bool Transliterate(std::string_view sv, int8_t langCode, std::string & out) const;
  std::string Transliterate(localisation::LanguageIndex const languageIndex, std::string const name) const;

  // Transliterates |str| with |transliteratorId|, ignores mode.
  bool TransliterateForce(std::string const & str, std::string const & transliteratorId, std::string & out) const;

private:
  Transliteration() = default;

  std::atomic<Mode> m_mode = Mode::Enabled;
};
