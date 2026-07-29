#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport;

int main() {
    TransportCatalogue catalogue;

    {
        InputReader reader;
        reader.ReadRequest(cin);
        reader.ApplyCommands(catalogue);
    }

    ProcessStatRequest(cin, cout, catalogue);

    return 0;
}