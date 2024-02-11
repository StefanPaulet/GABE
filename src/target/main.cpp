//
// Created by stefan on 2/8/24.
//

#include "utils/math/linearMatrix/LinearMatrix.hpp"
#include <iostream>
#include <utils/math/linearArray/LinearArray.hpp>

using namespace gabe::utils::math;

int main() {
  std::array<int, 3> a1 {{1, 2, 3}};
  LinearArray<int, 3> a {{1, 2, 3}};
  LinearArray<int, 3> b {{5, -2, 3}};
  LinearMatrix<int, 2, 3> c {{a, b}};
  LinearArray<int, 2, 3> d {{a + a, b}};

  LinearArray<int, 2, 3> {{LinearArray<int, 3> {{1, 2, 3}}, LinearArray<int, 3> {{5, -2, 3}}}};
  LinearArray<int, 1, 3> e1 {{ LinearArray<int, 3>{{1, 2, 3}} }};

  std::cout << c << '\n';
  std::cout << a << '\n';
}
