#include "indexer/transliteration_loader.hpp"

#include "platform/platform.hpp"

#include "i18n/transliteration.hpp"

void InitTransliterationInstanceWithDefaultDirs()
{
  Transliteration::Instance().Init(GetPlatform().ResourcesDir());
}
