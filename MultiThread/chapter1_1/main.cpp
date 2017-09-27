#include <thread>
#include <iostream>
using namespace std;
void hello()
{
    cout << "Hello\n";
}

void main()
{
    thread t(hello);
    //t.join();
    t.detach();
}