
#include "kul/log.hpp"

volatile size_t NOOP = 0;

const constexpr size_t SIZE = 9876543210;

class A{

public:
  virtual ~A(){}
  virtual void  act() = 0;
};

class B : public A{
public:  
  virtual void act(){
    NOOP += 1;
  }
};

class C{
public:
  virtual void act(){
    NOOP += 1;
  }
};

int main(int argc, char* argv[]){
  size_t i = 0;
  auto now = kul::Now::NANOS();

  now = kul::Now::NANOS();
  {
    B z;
    for(i = 0; i < SIZE; i++) z.act();
  }
  {
    C z;
    for(i = 0; i < SIZE; i++) z.act();
  }
  KLOG(INF) << "time for B+C: " << (kul::Now::NANOS() - now) / 1000000;
  return 0;
}
