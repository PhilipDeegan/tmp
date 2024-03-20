
#include "box.hpp"
#include <memory>

struct A {
    virtual ~A() {}

    virtual bool isIn(Point const& point, Box const& box) = 0;
};

struct B : public A {
    ~B() override {}

    bool isIn(Point const& point, Box const& box) override {
        bool pointInBox = true;
        for (auto iDim = 0u; iDim < dim; ++iDim)
            pointInBox =
                pointInBox & point[iDim] >= box.lower[iDim] && point[iDim] <= box.upper[iDim];
        return pointInBox;
    }
};

auto fn() {
    Box box{{0, 0, 0}, {9, 9, 9}};
    std::vector<Point> points(nPoints, defP);
    std::size_t count = 0;

    std::shared_ptr<A> a = std::make_shared<B>();
    auto const s = mkn::kul::Now::MILLIS();

    for (std::size_t i = 0; i < nTimes; ++i)
        for (auto const& p : points) count += a->isIn(p, box);

    std::cout << __FILE__ << " " << __LINE__ << " " << count << std::endl;
    auto const total = mkn::kul::Now::MILLIS() - s;
    KOUT(NON) << "RUN: " << total << " ms";
    KOUT(NON) << "AVG: " << (total / nTimes) << " ms";
}

int main() { fn(); }
