
#include "box.hpp"

bool isIn(Point const& p, Box const& box) {
    std::int16_t l, u;

    std::int16_t v = std::numeric_limits<std::int16_t>::max();
    for (auto iDim = 0u; iDim < dim; ++iDim) {
        l = p[iDim] - box.lower[iDim];
        u = box.upper[iDim] - p[iDim];
        v = std::min(v, l);
        v = std::min(v, u);
    }

    return v >= 0;
}

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
