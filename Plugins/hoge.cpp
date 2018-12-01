#include "Eigen/Core"
#include <iostream>

using namespace Eigen;
using namespace std;

int main()
{

    Vector3d vec(1,2,3);
    Vector3d vec2(3,4,5);

    // vec = vec.cwiseProduct(vec2);
    vec = vec.cwiseQuotient(vec2);

    cout << vec << endl;

    return 0;
}