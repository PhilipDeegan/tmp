
#include "kul/log.hpp"

volatile size_t NOOP = 0;

const constexpr size_t SIZE = 9876543210;

class A{
public:
  static void act(){
    NOOP += 1;
    NOOP -= 1;
  }
};

class B{

public:
  static void act(){
    NOOP += 1;
  }
};

template <class T>
class Z{
public:
  void act(){
    T::act();
  }
};


int main(int argc, char* argv[]){
  size_t i = 0;
  auto now = kul::Now::NANOS();

  now = kul::Now::NANOS();
  {
    Z<A> z;
    for(i = 0; i < SIZE; i++) z.act();
  }
  {
    Z<B> z;
    for(i = 0; i < SIZE; i++) z.act();
  }
  KLOG(INF) << "time for A+B: " << (kul::Now::NANOS() - now) / 1000000;
  return 0;
}
