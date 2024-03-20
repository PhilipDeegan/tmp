#include <vector>
#include <tuple>
#include <iostream>
#include <cstdint>
#include <array>

#include "mkn/kul/log.hpp"
#include "mkn/kul/time.hpp"

constexpr static std::size_t nTimes = 20;
constexpr static std::uint32_t dim = 3;
using Point = std::array<std::int32_t, dim>;      // 3 * 4 bytes
constexpr static std::size_t nPoints = 16000000;  // 5e8;   // 3 * 4 * 5e8 == 6GBs
constexpr static Point defP{5, 5, 5};

struct Box {
    Point lower;
    Point upper;
};

template <typename Type, std::size_t size>
constexpr std::array<Type, size> ConstArray(Type val = 0) {
    std::array<Type, size> arr{};
    for (uint8_t i = 0; i < size; i++) arr[i] = val;
    return arr;
}

template <size_t dim>
struct Particle {
    static_assert(dim > 0 and dim < 4, "Only dimensions 1,2,3 are supported.");
    static const size_t dimension = dim;

    Particle(double a_weight, double a_charge, std::array<int, dim> cell,
             std::array<double, dim> a_delta, std::array<double, 3> a_v)
        : weight{a_weight}, charge{a_charge}, iCell{cell}, delta{a_delta}, v{a_v} {}

    Particle() = default;

    double weight;
    double charge;

    std::array<int, dim> iCell = ConstArray<int, dim>();
    std::array<double, dim> delta = ConstArray<double, dim>();
    std::array<double, 3> v = ConstArray<double, 3>();

    double Ex = 0, Ey = 0, Ez = 0;
    double Bx = 0, By = 0, Bz = 0;
};
