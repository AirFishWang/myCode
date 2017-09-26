#include <iostream>
using namespace std;

class A{
public:
    ~A();
};

class B{
protected:
    ~B();
};

class C{
private:
    ~C();
};

void main()
{
    A a;
    B *l = new B;
    B b;
    C c;
}