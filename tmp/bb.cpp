#include <iostream>
#include <cstdint>
#include <vector>
#include <array>
#include <algorithm>
#include <limits>

static constexpr std::size_t dim = 3;
using Point = std::array<std::int16_t, dim>;

struct Box {
    Point lower, upper;
};

bool isOutside(Point const& point, Box const& box) {
    bool pointOutsideBox = false;
    for (auto iDim = 0u; iDim < dim; ++iDim) {
        pointOutsideBox |= (point[iDim] < box.lower[iDim]) | (point[iDim] > box.upper[iDim]);
    }
    return pointOutsideBox;
}

std::int16_t fn(Box const& box, Point const& p) {
    // positive == inside domain
    // negative == outside domain
    std::int16_t l, u;

    std::int16_t v = std::numeric_limits<std::int16_t>::max();
    for (auto iDim = 0u; iDim < dim; ++iDim) {
        l = p[iDim] - box.lower[iDim];
        u = box.upper[iDim] - p[iDim];
        v = std::min(v, l);
        v = std::min(v, u);
    }

    return v;
}

int main() {
    Box box{{0, 0, 0}, {6, 6, 6}};

    Point out{{1, 1, -1}};
    Point low{{0, 0, 0}};
    Point mid{{3, 3, 3}};
    Point hig{{6, 6, 6}};

    std::cout << __LINE__ << " " << fn(box, low) << std::endl;
    std::cout << __LINE__ << " " << fn(box, mid) << std::endl;
    std::cout << __LINE__ << " " << fn(box, hig) << std::endl;
    std::cout << __LINE__ << " " << fn(box, out) << std::endl;

    return 0;
}
