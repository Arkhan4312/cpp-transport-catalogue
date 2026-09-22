#include "geo.h"

#include <algorithm>
#include <cmath>
namespace transport {
constexpr double kPi = 3.1415926535;
constexpr double kDegToRad = kPi / 180.;
constexpr int kEarthRadiusMeters = 6371000;

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }

    double ratio = sin(from.lat * kDegToRad) * sin(to.lat * kDegToRad) +
                   cos(from.lat * kDegToRad) * cos(to.lat * kDegToRad) * cos(abs(from.lng - to.lng) * kDegToRad);
    ratio = std::clamp(ratio, -1.0, 1.0);
    return acos(ratio) * kEarthRadiusMeters;
}
}  // namespace transport