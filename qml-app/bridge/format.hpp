#pragma once

#include "storage/storage_defines.hpp"

#include <QString>

#include <cstdint>
#include <string>

// The keyboard's locale, as the search engine names locales; "en" when it has no categories.
std::string InputLocale();

QString FormatSize(uint64_t bytes);
QString FormatDuration(int seconds);
QString FormatClockIn(int seconds);
QString StatusName(storage::NodeStatus status);
