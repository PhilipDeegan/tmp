

#include <cassert>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iostream>


template<typename T = std::vector<std::size_t>>
struct iterator_impl
{
    using This  = iterator_impl<T>;
    auto static constexpr is_const      = std::is_const_v<T>;

    using value_type = std::decay_t<typename T::value_type>;
    using difference_type   = std::size_t;
    using iterator_category = std::forward_iterator_tag;
    using pointer           = value_type*;
    using reference         = value_type&;


    iterator_impl(T& particles_)
        : particles{particles_}
    {
    }

    iterator_impl(iterator_impl&& that)      = default;
    iterator_impl(iterator_impl const& that) = default;

    auto& operator++()
    {
        ++curr_pos;
        return *this;
    }

    auto& operator+=(std::size_t i)
    {
        curr_pos += i;
        return *this;
    }

    auto& operator--()
    {
        --curr_pos;
        return *this;
    }
    auto operator+(std::size_t i) const
    {
        auto copy = *this;
        copy.curr_pos += i;
        return copy;
    }
    auto operator-(std::size_t i) const
    {
        auto copy = *this;
        copy.curr_pos -= i;
        return copy;
    }



    auto operator-(This const& that) const
    {
        return curr_pos - that.curr_pos;
    }


    auto& operator=(iterator_impl const& that)
    {
        curr_pos = that.curr_pos;
        return *this;
    }
    auto& operator=(iterator_impl&& that)
    {
        curr_pos = that.curr_pos;
        return *this;
    }

    auto operator==(iterator_impl const& that) const { return curr_pos == that.curr_pos; }
    auto operator!=(iterator_impl const& that) const { return curr_pos != that.curr_pos; }
    auto operator<(iterator_impl const& that) const { return curr_pos < that.curr_pos; }

    auto& operator*() { return particles[curr_pos]; }
    auto& operator*() const { return particles[curr_pos]; }

    T& particles;
    std::size_t curr_pos = 0;
};

template<typename V>
auto _begin(V & v){
    return iterator_impl<V>(v);
}
template<typename V>
auto _end(V & v){
    return iterator_impl<V>(v) + v.size();
}


int main(){
    std::vector<std::size_t> v(5, 1);
    v[0] = 2;

    auto b = _begin(v);
    auto e = _end(v);

    for(auto const& i : v) std::cout << i << " ";
    std::cout << std::endl;

    std::sort(b, e);

    for(auto const& i : v) std::cout << i << " ";
    std::cout << std::endl;
}