#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include <vector>
#include <iostream>
#include <sstream>

constexpr std::uint16_t INTERP_GAP = 2;  // all directions
constexpr std::uint16_t CL_BYTES = 64;
std::mt19937_64 eng{std::random_device{}()};  // or seed however you want
std::uniform_int_distribution<> dist{100, 333};

struct B {
    std::array<std::uint32_t, 3> lower, upper;

    auto size() const {
        std::size_t s = upper[0] - lower[0] + 1;
        for (std::size_t i = 1; i < 3; ++i) s *= upper[i] - lower[i] + 1;
        return s;
    }
};

struct P {
    int v = 0;
    std::array<std::uint32_t, 3> iCell{0, 0, 0};

    void process() {  // pretend to do something
        std::this_thread::sleep_for(std::chrono::milliseconds{dist(eng)});
    };

    auto static in_box(B b, std::size_t n) {
        auto gen = [&]() {
            std::random_device rd;
            std::seed_seq seed_seq{rd(), rd(), rd(), rd(), rd(), rd(), rd()};
            return std::mt19937_64{seed_seq};
        }();
        std::vector<P> ps(n);
        for (std::size_t di = 0; di < 3; ++di) {
            std::uniform_int_distribution<> distrib(b.lower[di], b.upper[di]);
            for (auto& p : ps) p.iCell[di] = distrib(gen);
        }
        return ps;
    }

    auto& operator[](std::uint16_t const& i) { return iCell[i]; }
    auto& operator[](std::uint16_t const& i) const { return iCell[i]; }

    auto static constexpr size() { return 3; }

    auto to_string() const {
        std::stringstream ss;
        ss << "icell: [" << iCell[0] << ", " << iCell[1] << ", " << iCell[2] << "]";
        return ss.str();
    }
};

struct T {
    std::uint16_t pid = 0;
    std::array<std::uint32_t, 3> iCell{0, 0, 0};

    std::vector<std::array<std::uint32_t, 3>> distance_from_others;  // needs == pid

    auto initialize(std::uint16_t n) {
        std::vector<T> ts(n);

        return ts;
    }
};

struct Patch {
    Patch(B box, std::size_t n_particles = 1e6)
        : box{box}, particles{P::in_box(box, n_particles)} {}

    B box;
    std::vector<P> particles;
    // std::vector<uint16_t> particles_per_cell;
    void print() {
        for (auto const& p : particles) std::cout << p.to_string() << std::endl;
        std::cout << std::endl;
    }
};

template <typename Level>
auto initialize(Level const& level, std::uint16_t threads = 10) {
    std::vector<T> ts;
    std::vector<Patch*> sorted_by_n_particles(level.size());
    return ts;
}

template <typename V, std::int16_t S>
bool static constexpr el_wise_less(V const& v0, V const& v1) {
    // for (std::int16_t i = S - 1; i >= 0; --i) {
    //     if (v0[i] < v1[i]) return true;
    //     // if (v0[i] != v1[i]) return false;
    // }
    return v0.iCell < v1.iCell;
}

namespace std {
void sort(std::vector<P>& particles) {
    std::sort(particles.begin(), particles.end(), el_wise_less<P, 3>);
    // [](auto const& a, auto const& b) { return a.iCell < b.iCell; });
}
void sort(Patch& patch) { std::sort(patch.particles); }

}  // namespace std

void f() {
    Patch p{B{{0, 0, 0}, {8, 8, 8}}, 10};

    p.print();
    std::sort(p);
    p.print();
}

int main() { f(); }