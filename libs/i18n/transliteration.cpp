#include "i18n/transliteration.hpp"

#include "base/macros.hpp"

Transliteration & Transliteration::Instance()
{
  static Transliteration instance;
  return instance;
}

void Transliteration::Init(std::string const & icuDataDir)
{
  UNUSED_VALUE(icuDataDir);
}

void Transliteration::SetMode(Mode mode)
{
  m_mode = mode;
}

bool Transliteration::Transliterate(std::string_view, int8_t, std::string &) const
{
  return false;
}

std::string Transliteration::Transliterate(localisation::LanguageIndex const, std::string const name) const
{
  return name;
}

bool Transliteration::TransliterateForce(std::string const &, std::string const &, std::string &) const
{
  return false;
}
