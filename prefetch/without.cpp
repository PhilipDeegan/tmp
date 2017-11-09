
#include "random.hpp"
#include "kul/log.hpp"

using Random = effolkronium::random_static;

const constexpr size_t SIZE = 9876543210;

int main(int argc, char* argv[]){

    auto now = kul::Now::NANOS();

    double *d1 = new double[SIZE];
    double *d2 = new double[SIZE];

    for(size_t i = 0; i < SIZE; i++){
      d1[i] = i + ((i+i)*i);
    }






    size_t d1_i = 0;
    for(size_t i = 0; i < SIZE; i++){


      d1_i = d2[Random::get(0, (int)SIZE - 1)];
      d2[i] = 
        d1[d1_i] 
        +
            d1[d1_i]
            ;
    }

    delete[] d1;
    delete[] d2;

    KLOG(INF) << "time for : " << (kul::Now::NANOS() - now) / 1000000;

    return 0;
}
