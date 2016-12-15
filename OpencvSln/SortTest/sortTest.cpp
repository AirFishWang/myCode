#include <iostream>
#include <algorithm>
#include <vector> 
using namespace std;

int main()
{
    vector<int> array = {1,2,2,1,0};
    sort(array.begin(), array.end());
    for (size_t i = 0; i < array.size(); i++)
    {
        cout << array[i] << endl;
    }
}
