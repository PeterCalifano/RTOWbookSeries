#include <iostream>
#include <Eigen/Dense>
#include <gdt/math/vec.h>

using namespace gdt;

int main()
{   
    vec_t<float, 5> v1;

    // Init vector
    for (int i = 0; i < 5; i++)
        v1[i] = i;

    return 0;
}