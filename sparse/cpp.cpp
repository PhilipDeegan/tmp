
#include <atomic>

#include "random.hpp"
#include "kul/log.hpp"
#include "kul/time.hpp"
#include "kul/threads.hpp"

using Random = effolkronium::random_static;

const constexpr size_t SIZE = 128;

void EIGHT(const size_t i, double d[][SIZE], size_t& test){
  for(size_t k = 0; k < SIZE; k+=8){
    if(d[i][7 + k]){ test += i + 7 + k; }
    if(d[i][0 + k]){ test += i + 0 + k; }
    if(d[i][5 + k]){ test += i + 5 + k; }
    if(d[i][3 + k]){ test += i + 3 + k; }
    if(d[i][6 + k]){ test += i + 6 + k; }
    if(d[i][1 + k]){ test += i + 1 + k; }
    if(d[i][4 + k]){ test += i + 4 + k; }
    if(d[i][2 + k]){ test += i + 2 + k; }
  }
}

void SIX(const size_t i, double d[][SIZE], size_t& test){
  for(size_t k = 0; k < SIZE; k += 8){
    if(d[i][7 + k]){ test += i + 7 + k; }
    if(d[i][0 + k]){ test += i + 0 + k; }
    if(d[i][5 + k]){ test += i + 5 + k; }
    if(d[i][3 + k]){ test += i + 3 + k; }
    if(d[i][6 + k]){ test += i + 6 + k; }
    if(d[i][1 + k]){ test += i + 1 + k; }
  }
}

void FOUR(const size_t i, double d[][SIZE], size_t& test){
  for(size_t k = 0; k < SIZE; k+=8){
    if(d[i][7 + k]){ test += i + 7 + k; }
    if(d[i][0 + k]){ test += i + 0 + k; }
    if(d[i][5 + k]){ test += i + 5 + k; }
    if(d[i][3 + k]){ test += i + 3 + k; }
  }
}

void TWO(const size_t i, double d[][SIZE], size_t& test){
  for(size_t k = 0; k < SIZE; k+=8){
    if(d[i][5 + k]){ test += i + 5 + k; }
    if(d[i][3 + k]){ test += i + 3 + k; }
  }
}

void ONE(const size_t i, double d[][SIZE], size_t& test){
  for(size_t k = 0; k < SIZE; k+=8){
    if(d[i][3 + k]){ test += i + 3 + k; }
  }
}

void seq(const size_t i, double d[][SIZE], size_t& test){
  for(size_t j = 0; j < SIZE; j++) if(d[i][j]) test += i + j;
}

int main(int argc, char* argv[]){
  double d [SIZE * SIZE][SIZE]={0};

  size_t total = 0, test = 0;
  for(size_t i = 0; i < SIZE * SIZE; i++){
    while(true){
      auto c1 = Random::get(0, (int)SIZE - 1);
      auto c2 = Random::get(0, (int)SIZE - 1);
      if(d[c1][c2] == 0){
        d[c1][c2] = 1;
        total += c1 + c2;
        break;
      }
    }
  }

// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 
// 

  auto now = kul::Now::NANOS();

  now = kul::Now::NANOS();
  for(size_t i = 0; i < SIZE * SIZE; i++) EIGHT(i, d, test);
  KLOG(INF) << "time for EIGHT         : " << (kul::Now::NANOS() - now);
  KLOG(INF) << "found value            : " << (test);
  KLOG(INF);
  test = 0;

  now = kul::Now::NANOS();
  for(size_t i = 0; i < SIZE * SIZE; i++) SIX(i, d, test);
  KLOG(INF) << "time for SIX           : " << (kul::Now::NANOS() - now);
  KLOG(INF) << "found value            : " << (test);
  KLOG(INF);
  test = 0;

  now = kul::Now::NANOS();
  for(size_t i = 0; i < SIZE * SIZE; i++) FOUR(i, d, test);
  KLOG(INF) << "time for FOUR          : " << (kul::Now::NANOS() - now);
  KLOG(INF) << "found value            : " << (test);
  KLOG(INF);
  test = 0;

  now = kul::Now::NANOS();
  for(size_t i = 0; i < SIZE * SIZE; i++) TWO(i, d, test);
  KLOG(INF) << "time for TWO           : " << (kul::Now::NANOS() - now);
  KLOG(INF) << "found value            : " << (test);
  KLOG(INF);
  test = 0;

  now = kul::Now::NANOS();
  for(size_t i = 0; i < SIZE * SIZE; i++) ONE(i, d, test);
  KLOG(INF) << "time for ONE           : " << (kul::Now::NANOS() - now);
  KLOG(INF) << "found value            : " << (test);
  KLOG(INF);
  test = 0;

  now = kul::Now::NANOS();
  for(size_t i = 0; i < SIZE * SIZE; i++) seq(i, d, test);
  KLOG(INF) << "time for seq           : " << (kul::Now::NANOS() - now);
  KLOG(INF) << "found value            : " << (test);
  KLOG(INF);
  test = 0;

  {
    size_t n_threads = 8;
    size_t it = std::floor((SIZE * SIZE)/n_threads); // b = 16 | th = 2 | it = 8
    size_t ma = it * n_threads;
    std::atomic<size_t> atest(0);
    std::vector<std::thread> threads;
    now = kul::Now::NANOS();
    for(size_t i = 0; i < n_threads; i++)
      threads.emplace_back([&](const size_t s, const size_t m){
          size_t t = 0;
          for(uint i = (s * it); i < (s + it); i++) EIGHT(i, d, t);
          atest += t;
      }, i, ma);
    for (size_t i = 0; i < n_threads; i++) threads[i].join();
    KLOG(INF) << "time for threads EIGHT : " << (kul::Now::NANOS() - now);
    KLOG(INF) << "found value            : " << (atest);
    KLOG(INF);
  }

  {
    size_t n_threads = 8;
    size_t it = std::floor((SIZE * SIZE)/n_threads); // b = 16 | th = 2 | it = 8
    size_t ma = it * n_threads;
    std::atomic<size_t> atest(0);
    std::vector<std::thread> threads;
    now = kul::Now::NANOS();
    for(size_t i = 0; i < 8; i++)
      threads.emplace_back([&](const size_t s, const size_t m){
          size_t t = 0;
          for(uint i = (s * it); i < (s + it); i++) seq(i, d, t);
          atest += t;
      }, i, ma);
    for (size_t i = 0; i < 8; i++)
      threads[i].join();
    KLOG(INF) << "time for threads seq   : " << (kul::Now::NANOS() - now);
    KLOG(INF) << "found value            : " << (atest);
    KLOG(INF);
  }
  KLOG(INF) <<   "total                  : " << total;
  return 0;
}
