
#include <vector>
#include <tuple>
#include <iostream>
#include <cstdint>
#include <array>
#include <optional>
#include <random>

#include "mkn/kul/log.hpp"
#include "mkn/kul/time.hpp"

constexpr static std::size_t nTimes = 20;
constexpr static std::uint32_t dim = 3;
using Point = std::array<std::int32_t, dim>;  // 3 * 4 bytes
constexpr static std::size_t nPoints = 5e8;   // 3 * 4 * 5e8 == 6GBs
constexpr static Point defP{5, 5, 5};

struct Box {
    Point lower;
    Point upper;
};

bool isIn(Point const& point, Box const& box) {
    auto isIn1D = [](typename Point::value_type pos, typename Point::value_type lower,
                     typename Point::value_type upper) { return pos >= lower && pos <= upper; };

    bool pointInBox = true;

    for (auto iDim = 0u; iDim < dim; ++iDim) {
        pointInBox = pointInBox & isIn1D(point[iDim], box.lower[iDim], box.upper[iDim]);
    }
    return pointInBox;
}

template <typename Particles, typename Point>
void disperse(Particles& particles, Point lo, Point up, std::optional<int> seed = std::nullopt) {
    auto gen = [&]() {
        if (!seed.has_value()) {
            std::random_device rd;
            std::seed_seq seed_seq{rd(), rd(), rd(), rd(), rd(), rd(), rd()};
            return std::mt19937_64(seed_seq);
        }
        return std::mt19937_64(*seed);
    }();
    for (std::size_t i = 0; i < dim; i++) {
        std::uniform_int_distribution<> distrib(lo[i], up[i]);
        for (auto& particle : particles) particle[i] = distrib(gen);
    }
}

auto fn() {
    Box box{{0, 0, 0}, {9, 9, 9}};
    std::vector<Point> points(nPoints, defP);
    disperse(points, box.lower, box.upper, 13333337);

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
