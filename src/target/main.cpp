//
// Created by stefan on 2/8/24.
//

#include <iostream>
#include <utils/math/linearArray/LinearArray.hpp>

using namespace gabe::utils::math;

int main() {
  LinearArray<int, 3> a {{1, 2, 3}};
  LinearArray<int, 3> b {{5, -2, 3}};
  LinearArray<int, 2, 3> c {{a, b}};
  LinearArray<int, 2, 3> d {{a + a, b}};
  std::cout << a.dot(b);
}