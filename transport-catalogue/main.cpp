#include <iostream>
#include <string>

#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
using namespace std;
using namespace transport;

int main() {
    TransportCatalogue catalogue;
    RequestHandler handler(catalogue);
    JsonReader reader(catalogue, handler);

    reader.LoadData(std::cin);
    reader.ProcessRequests(std::cout);
    reader.MapToFile("map.svg");
    return 0;
}