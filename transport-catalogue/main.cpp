#include <iostream>

#include "json_reader.h"
#include "transport_catalogue.h"

int main() {
    transport::TransportCatalogue catalogue;
    transport::RequestHandler handler(catalogue);
    transport::JsonReader reader(catalogue, handler);

    reader.LoadData(std::cin);
    reader.ProcessRequests(std::cout);
    return 0;
}