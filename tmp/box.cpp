
#include "box.hpp"

template <typename T = std::uint16_t>
struct Apply {
    template <T i>
    constexpr auto inline operator()() {
        return std::integral_constant<T, i>{};
    }
};

template <typename Apply, std::uint16_t... Is>
constexpr auto inline apply_N(Apply& f, std::integer_sequence<std::uint16_t, Is...> const&) {
    if constexpr (!std::is_same_v<decltype(f.template operator()<0>()), void>)
        return std::make_tuple(f.template operator()<Is>()...);
    (f.template operator()<Is>(), ...);
}
template <std::uint16_t N, typename Apply>
constexpr auto inline apply_N(Apply&& f) {
    return apply_N(f, std::make_integer_sequence<std::uint16_t, N>{});
}

template <std::uint16_t N, typename Fn>
constexpr auto inline for_N(Fn& fn) {
    using return_type =
        std::decay_t<std::result_of_t<Fn(std::integral_constant<std::uint16_t, 0>)>>;
    constexpr bool returns = !std::is_same_v<return_type, void>;

    /*
        for_N<2>([](auto ic) {
            constexpr auto i = ic();
            // ...
        });
    */
    if constexpr (returns)
        return std::apply([&](auto... ics) { return std::make_tuple(fn(ics)...); },
                          apply_N<N>(Apply{}));
    else
        std::apply([&](auto... ics) { (fn(ics), ...); }, apply_N<N>(Apply{}));
}

template <std::uint16_t N, typename Fn>
constexpr auto inline for_N(Fn&& fn) {
    return for_N<N>(fn);
}

template <std::uint16_t N, typename Fn>
constexpr auto inline for_N_all(Fn&& fn) {
    return std::apply([&](auto const&... item) { return (item & ...); }, for_N<N>(fn));
}

template <std::uint16_t N, typename Fn>
constexpr auto inline for_N_any(Fn&& fn) {
    return std::apply([&](auto const&... item) { return (item | ...); }, for_N<N>(fn));
}

bool isIn(Point const& point, Box const& box) {
    return for_N_all<dim>([&](auto const iDim) {
        return point[iDim] >= box.lower[iDim] & point[iDim] <= box.upper[iDim];
    });
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

    KLOG(INF) << sizeof(Particle<dim>);
}

int main() { fn(); }
