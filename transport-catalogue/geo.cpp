#include "geo.h"
namespace transport {
double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    static const int kEarthRadius = 6371000;
    double ratio = sin(from.lat * dr) * sin(to.lat * dr) +
                   cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr);
    ratio = std::clamp(ratio, -1.0, 1.0);
    return acos(ratio) * kEarthRadius;
}
}  // namespace transport