
#include "kul/log.hpp"

volatile size_t NOOP = 0;

const constexpr size_t SIZE = 9876543210;

class A{
public:
  static void act(bool maybe){
    NOOP += 1;
    if(maybe){
      NOOP -= 1;
    }
  }
};

template <class T>
class Z{
public:
  void act(size_t i){
    T::act(i);
  }
};


int main(int argc, char* argv[]){
  size_t i = 0;
  auto now = kul::Now::NANOS();

  now = kul::Now::NANOS();
  bool no = 1;
  {
    Z<A> z;
    for(i = 0; i < SIZE; i++) z.act(no);
  }
  no = !no;
  {
    Z<A> z;
    for(i = 0; i < SIZE; i++) z.act(no);
  }
  KLOG(INF) << "time for A+B: " << (kul::Now::NANOS() - now) / 1000000;
  return 0;
}
