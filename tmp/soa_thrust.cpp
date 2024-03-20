
#include <thrust/sort.h>
#include <thrust/gather.h>
#include <thrust/sequence.h>
#include <thrust/functional.h>
#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>

#include "mkn/gpu.hpp"

#include <array>
#include <vector>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <algorithm>

#define _DEV_FN_ __device__
#define _HST_FN_ __host__
#define _ALL_FN_ _HST_FN_ _DEV_FN_

#define PRINT(x) std::cout << __LINE__ << " " << x << std::endl;
#define abort_if(x)                         \
    if (x) {                                \
        std::cout << __LINE__ << std::endl; \
        std::abort();                       \
    }

template <typename T>
using ManagedVector = std::vector<T, mkn::gpu::ManagedAllocator<T>>;

template <typename ParticleArray_t>
struct ParticleIteratorAdapter;
template <typename ParticleArray_t>
struct ParticleArrayIterator;

struct P {
    P() _ALL_FN_ {}
    P(std::array<int, 3> const& ic) : iCell_{ic} {}

    template <typename ParticleArray_t>
    _ALL_FN_ P(ParticleIteratorAdapter<ParticleArray_t> const& that);

    auto& iCell() _ALL_FN_ { return iCell_; }
    auto& iCell() const _ALL_FN_ { return iCell_; }

    std::array<int, 3> iCell_{100, 100, 100};
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
    RValue operator()(Icell const& icell) const _ALL_FN_ {
        if constexpr (Box_t::dimension == 2) return icell[1] + icell[0] * shape[1] * shape[0];
        if constexpr (Box_t::dimension == 3)
            return icell[2] + icell[1] * shape[2] + icell[0] * shape[1] * shape[2];
        return icell[0];
    }
    Box_t const box;
    std::array<int, Box_t::dimension> shape = box.shape();
};

struct ParticleArrayView {
    using This = ParticleArrayView;
    // void push_back(std::array<int, 3> const& i) { iCells.push_back(i); }
    // void swap(std::size_t const& a, std::size_t const& b) {
    //     if (a == b) return;
    //     std::swap(iCells[a], iCells[b]);
    // }
    auto size() const { return size_; }
    ParticleArrayIterator<This> begin();
    ParticleArrayIterator<This> end();

    // template <typename CF>
    // void print(CF const& flattener) {
    //     for (auto const& iCell : iCells)
    //         std::cout << __LINE__ << " " << flattener(iCell) << std::endl;
    //     std::cout << std::endl;
    // }

    std::array<int, 3>* iCells;
    std::size_t size_;
};

struct ParticleArrayViewMediator {
    using This = ParticleArrayViewMediator;

    ParticleArrayView view;
    ManagedVector<ParticleArrayView> views{view};  // hax

    ParticleArrayIterator<ParticleArrayView> begin();
    ParticleArrayIterator<ParticleArrayView> end();
};

struct ParticleArray {
    using This = ParticleArray;

    void push_back(std::array<int, 3> const& i) { iCells.push_back(i); }
    void swap(std::size_t const& a, std::size_t const& b) {
        if (a == b) return;
        std::swap(iCells[a], iCells[b]);
    }
    auto size() const { return iCells.size(); }
    ParticleArrayIterator<This> begin();
    ParticleArrayIterator<This> end();

    template <typename CF>
    void print(CF const& flattener) {
        for (auto const& iCell : iCells)
            std::cout << __LINE__ << " " << flattener(iCell) << std::endl;
        std::cout << std::endl;
    }

    auto view() { return ParticleArrayViewMediator{{iCells.data(), size()}}; }

    ManagedVector<std::array<int, 3>> iCells;
};

template <typename ParticleArray_t>
struct ParticleIteratorAdapter {  // dereferencing adapter
    using This = ParticleIteratorAdapter;

    ParticleIteratorAdapter() = delete;
    ParticleIteratorAdapter(ParticleArray_t* ps_, std::size_t index)
        : ps{ps_}, index_{index}, ref{1} {
        // printf("%s %i %lu %lu \n", __FILE__, __LINE__, index_, ref);
        copy.iCell() = ps->iCells[index_];
    }
    ParticleIteratorAdapter(ParticleIteratorAdapter const& that) _ALL_FN_ : ps{that.ps},
                                                                            index_{that.index_},
                                                                            copy{that.copy} /*,
                                                                             ref{that.ref}*/
    {
        if (that.ref) copy = ps->iCells[that.index_];
        // printf("%s %i %lu %lu \n", __FILE__, __LINE__, index_, that.ref);
    }
    ParticleIteratorAdapter(ParticleIteratorAdapter&&) = default;

    auto& operator=(P const& that) _ALL_FN_ {
        // ps->iCells[index_] = that.iCell();
        // printf("%s %i %lu %lu \n", __FILE__, __LINE__, index_, ref);

        if (ref == 1) {
            ps->iCells[index_] = that.iCell();
        } else
            copy.iCell() = that.iCell();
        return *this;
    }

    This& operator=(This&& that) _ALL_FN_ { return *this = that; }
    This& operator=(This const& that) _ALL_FN_ {
        // printf("%s %i %lu %lu \n", __FILE__, __LINE__, ref, that.ref);
        // printf("%s %i %lu %lu \n", __FILE__, __LINE__, index_, that.index_);

        if (ref) {
            if (that.ref)
                ps->iCells[index_] = ps->iCells[that.index_];
            else
                ps->iCells[index_] = that.copy.iCell();
        } else {
            if (that.ref)
                copy.iCell() = ps->iCells[that.index_];
            else
                copy.iCell() = that.copy.iCell();
        }

        // if (ref)  // ps->iCells[index_] = that.copy.iCell();
        //     ps->iCells[index_] = ps->iCells[that.index_];
        // else
        //     copy.iCell() = ps->iCells[that.index_];
        return *this;
    }

    auto& iCell() { return ref ? ps->iCells[index_] : copy.iCell(); }
    auto& iCell() const _ALL_FN_ { return ref ? ps->iCells[index_] : copy.iCell(); }

    auto& set_ref(std::size_t index) _ALL_FN_ {
        // printf("%s %i %lu %lu \n", __FILE__, __LINE__, index_, ref);
        ref = 1;
        index_ = index;
        copy.iCell() = ps->iCells[index_];
        return *this;
    }

    ParticleArray_t* ps = nullptr;
    std::size_t index_ = 50;
    P copy;
    bool ref = 0;  // 0=copy, 1=ref
};

template <typename ParticleArray_t>
P::P(ParticleIteratorAdapter<ParticleArray_t> const& that) _ALL_FN_ : iCell_{that.iCell()} {}

template <typename ParticleArray_t>
struct ParticleArrayIterator {
    using This = ParticleArrayIterator;
    using difference_type = std::size_t;
    using value_type = P;
    using reference = ParticleIteratorAdapter<ParticleArray_t>&;
    using pointer = ParticleIteratorAdapter<ParticleArray_t>*;
    using iterator_category = std::random_access_iterator_tag;

    ParticleArrayIterator() = delete;
    ParticleArrayIterator(ParticleArray_t* ps, std::size_t i = 0) _ALL_FN_ : particles{ps},
                                                                             index_{i} {};
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

    auto operator+(std::int64_t i) const _ALL_FN_ {
        auto copy = *this;
        copy.index_ += i;
        copy.scratch.set_ref(copy.index_);
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
        return particles == that.particles and index_ == that.index_;
    }
    auto operator!=(This const& that) const { return !(*this == that); }
    auto& operator*() const _ALL_FN_ { return scratch.set_ref(index_); }
    auto& operator[](std::size_t i) _ALL_FN_ { return scratch.set_ref(i); }
    auto& operator[](std::size_t i) const _ALL_FN_ { return scratch.set_ref(i); }

    auto operator<(This const& that) const { return index_ < that.index_; }

    ParticleArray_t* particles;
    std::size_t index_ = 0;

    // PRIVATE HANDS OFF

    mutable ParticleIteratorAdapter<ParticleArray_t> scratch{particles, 0};
};

ParticleArrayIterator<ParticleArray> ParticleArray::begin() {
    return ParticleArrayIterator<ParticleArray>{this};
}
ParticleArrayIterator<ParticleArray> ParticleArray::end() {
    return ParticleArrayIterator<ParticleArray>{this, size()};
}

ParticleArrayIterator<ParticleArrayView> ParticleArrayViewMediator::begin() {
    return ParticleArrayIterator<ParticleArrayView>{&views[0]};
}
ParticleArrayIterator<ParticleArrayView> ParticleArrayViewMediator::end() {
    return ParticleArrayIterator<ParticleArrayView>{&views[0], view.size()};
}

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

    // std::sort(particles.begin(), particles.end(),
    //           [&](auto const& a, auto const& b) { return cf(a.iCell()) < cf(b.iCell()); });

    auto ps = particles.view();
    auto v = &ps.views[0];
    // auto b = ps.begin();

    PRINT(cf(ps.views[0].iCells[0]));
    PRINT("");

    // thrust::sort(thrust::device, ps.begin(), ps.end(),
    //              [cf = cf] _DEV_FN_(auto const& a, auto const& b) {
    //                  printf("%s %i %lu %lu \n", __FILE__, __LINE__, cf(a.iCell()),
    //                  cf(b.iCell())); return cf(a.iCell()) < cf(b.iCell());
    //              });

    std::size_t l = 0;
    std::size_t r = iCells.size();
    ManagedVector<int> flats(r - l);
    auto fv = flats.data();
    mkn::gpu::GDLauncher{flats.size()}([=, cf = cf] _ALL_FN_() {
        auto idx = mkn::gpu::idx() + l;
        fv[mkn::gpu::idx()] = cf(v->iCells[idx]);  // cf(ps.iCell(idx));
    });
    thrust::device_vector<int> indices(flats.size());
    thrust::sequence(indices.begin(), indices.end());
    thrust::sort_by_key(thrust::device, fv, fv + flats.size(), indices.begin());
    thrust::gather(thrust::device, indices.begin(), indices.end(), ps.begin() + l, ps.begin() + l);

    particles.print(cf);

    for (std::size_t i = 0; i < particles.size(); ++i)
        if (particles.iCells[i] != expected[i]) return 1;
}
