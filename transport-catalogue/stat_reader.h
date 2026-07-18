#pragma once

#include <iosfwd>
#include "transport_catalogue.h"
namespace transport {
void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);
} // namespace transport