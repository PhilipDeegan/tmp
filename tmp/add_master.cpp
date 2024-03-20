
#include "add.hpp"

auto local_(Point const& cell, Box const& box)
{
    auto loc{cell};
    for (std::size_t i = 0; i < loc.size(); ++i)
        loc[i] -= box.lower[i];

    return loc;
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
        for (auto const& p : points) count += isIn(local_(p, box), box);

    std::cout << __FILE__ << " " << __LINE__ << " " << count << std::endl;
    auto const total = mkn::kul::Now::MILLIS() - s;
    KOUT(NON) << "RUN: " << total << " ms";
    KOUT(NON) << "AVG: " << (total / nTimes) << " ms";
}

int main() { fn(); }



