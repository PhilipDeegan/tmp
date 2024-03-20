#include <array>
#include <vector>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <algorithm>

#define PRINT(x) std::cout << __LINE__ << " " << x << std::endl;
#define abort_if(x)                         \
    if (x) {                                \
        std::cout << __LINE__ << std::endl; \
        std::abort();                       \
    }

struct ParticleIteratorAdapter;
struct ParticleArrayIterator;

struct P {
    P() {}
    P(std::array<int, 3> const& ic) : iCell_{ic} {}
    P(ParticleIteratorAdapter const& that);

    auto& iCell() { return iCell_; }
    auto& iCell() const { return iCell_; }

    std::array<int, 3> iCell_;
};

template <std::size_t dim>
struct Box {
    auto constexpr static dimension = dim;

    std::array<int, dim> lower;
    std::array<int, dim> upper;
    auto shape() const {
        std::array<int, dim> s;
        for (std::uint16_t i = 0; i < dim; ++i) s[i] = upper[i] - lower[i] + i;
        return s;
    };
};

template <typename Box_t, typename RValue = std::size_t>
struct CellFlattener {
    template <typename Icell>
    RValue operator()(Icell const& icell) const {
        if constexpr (Box_t::dimension == 2) return icell[1] + icell[0] * shape[1] * shape[0];
        if constexpr (Box_t::dimension == 3)
            return icell[2] + icell[1] * shape[2] + icell[0] * shape[1] * shape[2];
        return icell[0];
    }
    Box_t const box;
    std::array<int, Box_t::dimension> shape = box.shape();
};

struct ParticleArray {
    std::vector<std::array<int, 3>> iCells;
    void push_back(std::array<int, 3> const& i) { iCells.push_back(i); }
    void swap(std::size_t const& a, std::size_t const& b) {
        if (a == b) return;
        std::swap(iCells[a], iCells[b]);
    }
    auto size() const { return iCells.size(); }
    ParticleArrayIterator begin();
    ParticleArrayIterator end();

    template <typename CF>
    void print(CF const& flattener) {
        for (auto const& iCell : iCells)
            std::cout << __LINE__ << " " << flattener(iCell) << std::endl;
        std::cout << std::endl;
    }
};

struct ParticleIteratorAdapter {  // dereferencing adapter
    using This = ParticleIteratorAdapter;

    ParticleIteratorAdapter() = delete;
    ParticleIteratorAdapter(ParticleArray* ps_, std::size_t index) : ps{ps_}, index_{index} {}
    ParticleIteratorAdapter(ParticleIteratorAdapter const& that)
        : ps{that.ps}, index_{that.index_} {}
    ParticleIteratorAdapter(ParticleIteratorAdapter&&) = default;

    auto& operator=(P const& that) {
        ps->iCells[index_] = that.iCell();
        return *this;
    }

    This& operator=(This&& that) { return *this = that; }
    This& operator=(This const& that) {
        if (ref == 1) {
            ps->iCells[index_] = ps->iCells[that.index_];
        } else
            copy.iCell() = ps->iCells[that.index_];
        return *this;
    }

    auto& iCell() { return ps->iCells[index_]; }
    auto& iCell() const { return ps->iCells[index_]; }

    auto& set_ref(std::size_t index) {
        ref = 1;
        index_ = index;
        return *this;
    }

    ParticleArray* ps = nullptr;
    std::size_t index_;
    P copy;
    bool ref = 0;  // 0=copy, 1=ref
};

P::P(ParticleIteratorAdapter const& that) : iCell_{that.iCell()} {}

struct ParticleArrayIterator {
    using This = ParticleArrayIterator;
    using difference_type = std::size_t;
    using value_type = P;
    using reference = ParticleIteratorAdapter&;
    using pointer = ParticleIteratorAdapter*;
    using iterator_category = std::random_access_iterator_tag;

    ParticleArrayIterator() = delete;
    ParticleArrayIterator(ParticleArray& ps, std::size_t i = 0) : particles{ps}, index_{i} {};
    ParticleArrayIterator(This&& that) = default;
    ParticleArrayIterator(This const& that) = default;

    auto& operator=(This const& that) {
        index_ = that.index_;
        return *this;
    }
    auto& operator=(This&& that) {
        index_ = that.index_;
        return *this;
    }

    auto& operator++() {
        ++index_;
        return *this;
    }
    auto& operator+=(std::int64_t i) {
        index_ += i;
        return *this;
    }

    auto operator+(std::int64_t i) const {
        auto copy = *this;
        copy.index_ += i;
        return copy;
    }

    auto& operator--() {
        --index_;
        return *this;
    }
    auto operator-(This const& that) const { return index_ - that.index_; }
    auto operator-(std::int64_t i) const {
        auto copy = *this;
        copy.index_ -= i;
        return copy;
    }
    auto operator==(This const& that) const {
        return &particles == &that.particles and index_ == that.index_;
    }
    auto operator!=(This const& that) const { return !(*this == that); }
    auto& operator*() const { return scratch.set_ref(index_); }
    auto operator<(This const& that) const { return index_ < that.index_; }

    ParticleArray& particles;
    std::size_t index_ = 0;

    // PRIVATE HANDS OFF
    mutable ParticleIteratorAdapter scratch{&particles, 0};
};

ParticleArrayIterator ParticleArray::begin() { return ParticleArrayIterator{*this}; }
ParticleArrayIterator ParticleArray::end() { return ParticleArrayIterator{*this, size()}; }

std::vector<std::array<int, 3>> iCells{{2, 2, 2}, {0, 0, 0}, {0, 0, 0},  //
                                       {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
                                       {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
std::vector<std::array<int, 3>> expected{{0, 0, 0}, {0, 0, 0}, {0, 0, 0},  //
                                         {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
                                         {0, 0, 0}, {0, 0, 0}, {2, 2, 2}};

// std::vector<std::array<int, 3>> iCells{{2, 2, 2}, {2, 1, 0}, {1, 1, 1},  //
//                                        {1, 0, 1}, {0, 2, 0}, {0, 0, 0},
//                                        {1, 1, 2}, {0, 1, 0}, {2, 0, 2}};
// std::vector<std::array<int, 3>> expected{{0, 0, 0}, {0, 1, 0}, {0, 2, 0},  //
//                                          {1, 0, 1}, {1, 1, 1}, {1, 1, 2},
//                                          {2, 0, 2}, {2, 1, 0}, {2, 2, 2}};

int main() {
    using box_t = Box<3>;
    ParticleArray particles;
    box_t domain{{0, 0, 0}, {2, 2, 2}};
    CellFlattener<box_t> cf{domain};
    for (auto const& iCell : iCells) particles.push_back(iCell);

    particles.print(cf);

    std::sort(particles.begin(), particles.end(),
              [&](auto const& a, auto const& b) { return cf(a.iCell()) < cf(b.iCell()); });

    particles.print(cf);

    for (std::size_t i = 0; i < particles.size(); ++i)
        if (particles.iCells[i] != expected[i]) return 1;
}
