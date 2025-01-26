#include "2d_array.hpp"
#include <iostream>

int main()
{
    array_2d<int> arr;

    arr.add_row();
    arr[0].append(1);
    arr[0].append(2);
    arr.add_row();
    arr[1].append(1);
    arr[1].append(2);

    auto i = arr[0].begin();

    do {
        i = arr[0].next(i);
        std::cout << i.value << '\n';
    } while (i != arr[0].end());
}
