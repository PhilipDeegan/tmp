#include <cstddef>
#include <vector>
#include <omp.h>
#include <random>
#include <string>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <fstream>


class Timer

{
public:
    Timer(std::size_t& save, bool print = false)
        : tstart_{std::chrono::high_resolution_clock::now()}
        , save_{save}
        , print_{print}
    {
    }

    ~Timer()
    {
        tstop_ = std::chrono::high_resolution_clock::now();
        std::size_t duration
            = std::chrono::duration_cast<std::chrono::microseconds>(tstop_ - tstart_).count();
        save_ = duration;
        if (print_)
            std::cout << duration << " us\n";
    }

private:
    std::chrono::high_resolution_clock::time_point tstart_, tstop_;
    std::size_t& save_;
    bool print_;
};




template<std::size_t dim>
struct Box
{
    Box(std::array<std::size_t, dim> lower_, std::array<std::size_t, dim> upper_)
        : lower{lower_}
        , upper{upper_}
    {
    }
    std::array<std::size_t, dim> lower;
    std::array<std::size_t, dim> upper;
    auto size() const
    {
        std::size_t s = 1;
        for (std::size_t i = 0; i < dim; ++i)
        {
            s *= upper[i] - lower[i] + 1;
        }
        return s;
    }
    auto primal_size() const
    {
        std::size_t s = 1;
        for (std::size_t i = 0; i < dim; ++i)
        {
            s *= upper[i] - lower[i] + 1 + 1;
        }
        return s;
    }
};
template<std::size_t dim>
struct ThreadBox : Box<dim>
{
    ThreadBox(std::array<std::size_t, dim> lower_, std::array<std::size_t, dim> upper_)
        : Box<dim>(lower_, upper_)
        , density(this->primal_size())
        , fluxx(this->primal_size())
        , fluxy(this->primal_size())
        , fluxz(this->primal_size())
    {
    }
    std::vector<double> density;
    std::vector<double> fluxx;
    std::vector<double> fluxy;
    std::vector<double> fluxz;
};

template<std::size_t dim>
struct ParticleArray
{
};

template<>
struct ParticleArray<1>
{
    explicit ParticleArray(std::size_t nbparts)
        : icell_x(nbparts)
        , delta_x(nbparts)
    {
    }
    std::vector<int> icell_x;
    std::vector<double> delta_x;
};


template<>
struct ParticleArray<2>
{
    explicit ParticleArray(std::size_t nbparts)
        : icell_x(nbparts)
        , icell_y(nbparts)
        , delta_x(nbparts)
        , delta_y(nbparts)
    {
    }
    std::vector<int> icell_x;
    std::vector<int> icell_y;
    std::vector<double> delta_x;
    std::vector<double> delta_y;
};

template<>
struct ParticleArray<3>
{
    explicit ParticleArray(std::size_t nbparts)
        : icell_x(nbparts)
        , icell_y(nbparts)
        , icell_z(nbparts)
        , delta_x(nbparts)
        , delta_y(nbparts)
        , delta_z(nbparts)
    {
    }
    std::vector<int> icell_x;
    std::vector<int> icell_y;
    std::vector<int> icell_z;
    std::vector<double> delta_x;
    std::vector<double> delta_y;
    std::vector<double> delta_z;
};



template<std::size_t dim>
auto load_particles_random(Box<dim> const& box, std::size_t nppc)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distx(box.lower[0], box.upper[0]);
    std::uniform_int_distribution<> distdelta(0, 1);
    ParticleArray<dim> particles(nppc * box.size());
    for (std::size_t ip = 0; ip < particles.icell_x.size(); ++ip)
    {
        particles.icell_x[ip] = distx(gen);
        particles.delta_x[ip] = distdelta(gen);
    }
    if constexpr (dim >= 2)
    {
        std::uniform_int_distribution<int> disty(box.lower[1], box.upper[1]);
        for (std::size_t ip = 0; ip < particles.icell_x.size(); ++ip)
        {
            particles.icell_y[ip] = disty(gen);
            particles.delta_y[ip] = distdelta(gen);
        }
    }
    if constexpr (dim == 3)
    {
        std::uniform_int_distribution<int> distz(box.lower[2], box.upper[2]);
        for (std::size_t ip = 0; ip < particles.icell_x.size(); ++ip)
        {
            particles.icell_z[ip] = distz(gen);
            particles.delta_z[ip] = distdelta(gen);
        }
    }
    return particles;
}

template<std::size_t dim>
auto load_particles_ordered(Box<dim> const& box, std::size_t nppc)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distdelta(0, 1);
    ParticleArray<dim> particles(nppc * box.size());
    std::size_t pidx = 0;
    if constexpr (dim == 1)
    {
        for (std::size_t i = box.lower[0]; i <= box.upper[0]; ++i)
        {
            for (std::size_t ip = 0; ip < nppc; ++ip)
            {
                particles.icell_x[pidx]   = i;
                particles.delta_x[pidx++] = distdelta(gen);
            }
        }
        return particles;
    }
    else if constexpr (dim == 2)
    {
        for (std::size_t i = box.lower[0]; i <= box.upper[0]; ++i)
        {
            for (std::size_t j = box.lower[1]; j <= box.upper[1]; ++j)
            {
                for (std::size_t ip = 0; ip < nppc; ++ip)
                {
                    particles.icell_x[pidx]   = i;
                    particles.icell_y[pidx]   = j;
                    particles.delta_x[pidx]   = distdelta(gen);
                    particles.delta_y[pidx++] = distdelta(gen);
                }
            }
        }
        return particles;
    }
    else if constexpr (dim == 3)
    {
        for (std::size_t i = box.lower[0]; i <= box.upper[0]; ++i)
        {
            for (std::size_t j = box.lower[1]; j <= box.upper[1]; ++j)
            {
                for (std::size_t k = box.lower[2]; k <= box.upper[2]; ++k)
                {
                    for (std::size_t ip = 0; ip < nppc; ++ip)
                    {
                        particles.icell_x[pidx]   = i;
                        particles.icell_y[pidx]   = j;
                        particles.icell_z[pidx]   = k;
                        particles.delta_x[pidx]   = distdelta(gen);
                        particles.delta_y[pidx]   = distdelta(gen);
                        particles.delta_z[pidx++] = distdelta(gen);
                    }
                }
            }
        }
        return particles;
    }
}

template<std::size_t dim>
void deposit(ParticleArray<dim> const& particles, ThreadBox<dim>& threadbox)
{
    if constexpr (dim == 1)
        for (std::size_t ip = 0; ip < particles.icell_x.size(); ++ip)
        {
            auto dx = particles.delta_x[ip];
            auto ix = particles.icell_x[ip];
            auto nx = threadbox.upper[0] - threadbox.loxer[0] + 1;

            auto w1 = (1.0 - dx);
            auto w2 = (dx);

            auto ix1 = ix;
            auto ix2 = ix + 1;

            threadbox.density[ix1] += w1;
            threadbox.density[ix2] += w2;
        }
    if constexpr (dim == 2)
    {
        for (std::size_t ip = 0; ip < particles.icell_x.size(); ++ip)
        {
            auto dx = particles.delta_x[ip];
            auto dy = particles.delta_y[ip];
            auto ix = particles.icell_x[ip] - threadbox.lower[0];
            auto iy = particles.icell_y[ip] - threadbox.lower[1];
            auto nx = threadbox.upper[0] - threadbox.lower[0] + 2;
            auto ny = threadbox.upper[1] - threadbox.lower[1] + 2;

            auto w1 = (1.0 - dx) * (1.0 - dy);
            auto w2 = (1.0 - dx) * (dy);
            auto w3 = (dx) * (dy);
            auto w4 = (dx) * (1.0 - dy);

            auto ixy1 = iy + (ix)*ny;
            auto ixy2 = iy + 1 + (ix)*ny;
            auto ixy3 = iy + 1 + (ix + 1) * ny;
            auto ixy4 = iy + (ix + 1) * ny;

            threadbox.density[ixy1] += w1;
            threadbox.density[ixy2] += w2;
            threadbox.density[ixy3] += w3;
            threadbox.density[ixy4] += w4;

            threadbox.fluxx[ixy1] += w1;
            threadbox.fluxx[ixy2] += w2;
            threadbox.fluxx[ixy3] += w3;
            threadbox.fluxx[ixy4] += w4;

            threadbox.fluxy[ixy1] += w1;
            threadbox.fluxy[ixy2] += w2;
            threadbox.fluxy[ixy3] += w3;
            threadbox.fluxy[ixy4] += w4;

            threadbox.fluxz[ixy1] += w1;
            threadbox.fluxz[ixy2] += w2;
            threadbox.fluxz[ixy3] += w3;
            threadbox.fluxz[ixy4] += w4;
        }
    }
}


int main(int argc, char** argv)
{
    std::size_t N         = std::atoi(argv[1]);
    std::size_t TB        = std::atoi(argv[2]);
    std::size_t step      = TB;
    std::size_t LASTSTART = N;
    std::cout << N << "x" << N << "domain\n";
    std::cout << TB << "x" << TB << " threadboxes\n";
    double tseq;
    double tpar;
    constexpr std::size_t repeat = 1000;
    std::vector<double> times(repeat);
    for (std::size_t r = 0; r < repeat; ++r)
    {
        ThreadBox<2> domain{{0, 0}, {N, N}};
        auto particles = load_particles_ordered(domain, 10000);
        // Timer time(times[r]);
        auto start = omp_get_wtime();
        {
            deposit<2>(particles, domain);
        }
        times[r] = omp_get_wtime() - start;
    }
    tseq = std::accumulate(std::begin(times), std::end(times), 0., std::plus<double>());
    tseq /= static_cast<double>(repeat);
    std::cout << "sequential time : " << tseq << "\n";


    std::vector<ThreadBox<2>> boxes;
    std::vector<ParticleArray<2>> particles;
    for (std::size_t istart = 0; istart < LASTSTART; istart += step)
    {
        for (std::size_t jstart = 0; jstart < LASTSTART; jstart += step)
        {
            auto box = ThreadBox<2>{{istart, jstart}, {istart + TB - 1, jstart + TB - 1}};
            // std::cout << istart << "," << jstart << "-->" << istart + TB - 1 << ","
            //        << jstart + TB - 1 << "\n";
            boxes.push_back(box);
            particles.push_back(load_particles_ordered(box, 10000));
        }
    }
    std::cout << "there are " << boxes.size() << " threadboxes\n";
    std::cout << "there are " << particles.size() << " particle arrays\n";

    times.clear();
    times.resize(repeat);
    std::vector<int> nthreads{2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                              14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 23, 24};
    std::vector<double> speedup(nthreads.size());
    for (std::size_t ithread = 0; ithread < nthreads.size(); ++ithread)
    {
        omp_set_num_threads(nthreads[ithread]);
        for (std::size_t r = 0; r < repeat; ++r)
        {
            // Timer time(times[r]);
            auto start = omp_get_wtime();
#pragma omp parallel
            {
#pragma omp for
                for (std::size_t ibox = 0; ibox < boxes.size(); ++ibox)
                {
                    deposit<2>(particles[ibox], boxes[ibox]);
                }
            }
            times[r] = omp_get_wtime() - start;
        }
        tpar = std::accumulate(std::begin(times), std::end(times), 0., std::plus<double>());
        tpar /= static_cast<double>(repeat);
        auto su          = static_cast<double>(tseq) / tpar;
        speedup[ithread] = su;
        // std::cout << "parallel time with " << nthreads[ithread] << " threads : " << tpar << "\n";
        // std::cout << "speedup : " << su << '\n';
    }

    std::ofstream of{"speedup_" + std::to_string(N) + "_" + std::to_string(TB) + ".txt"};
    for (std::size_t ithread = 0; ithread < nthreads.size(); ++ithread)
    {
        of << nthreads[ithread] << " " << speedup[ithread] << "\n";
    }
    of.close();
    return 0;
}

