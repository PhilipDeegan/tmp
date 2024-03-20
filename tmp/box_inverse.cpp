
#include "box.hpp"

bool isIn(Point const& point, Box const& box) {
    bool pointOutsideBox = false;
    for (auto iDim = 0u; iDim < dim; ++iDim)
        pointOutsideBox |= point[iDim]<box.lower[iDim] | point[iDim]> box.upper[iDim];
    return !pointOutsideBox;
}

//

//

//
//
//

auto fn() {
    Box box{{0, 0, 0}, {9, 9, 9}};
    std::vector<Point> points(nPoints, defP);
    std::size_t count = 0;

    auto const s = mkn::kul::Now::MILLIS();

    for (std::size_t i = 0; i < nTimes; ++i)
        for (auto const& p : points) count += isIn(p, box);

    std::cout << __FILE__ << " " << __LINE__ << " " << count << std::endl;
    auto const total = mkn::kul::Now::MILLIS() - s;
    KOUT(NON) << "RUN: " << total << " ms";
    KOUT(NON) << "AVG: " << (total / nTimes) << " ms";
}

int main() { fn(); }
