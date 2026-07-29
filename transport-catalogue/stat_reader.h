#pragma once

#include <iosfwd>

#include "transport_catalogue.h"
namespace transport {
void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);

void ProcessStatRequest(std::istream& input, std::ostream& output, const TransportCatalogue& catalogue);
}  // namespace transportx