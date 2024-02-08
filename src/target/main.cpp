//
// Created by stefan on 2/8/24.
//

#include <iostream>
#include <utils/math/linearArray/LinearArray.hpp>

using namespace gabe::utils::math;

int main() {
  std::array<int, 3> a1 {{1, 2, 3}};
  LinearArray<int, 3> a {{1, 2, 3}};
  LinearArray<int, 3> b {{5, -2, 3}};
  LinearArray<int, 2, 3> c {{a, b}};
  LinearArray<int, 2, 3> d {{a + a, b}};
  auto f = c + d;

  // LinearArray<int, 2, 3> e2 {{ {1, 2, 3}, {5, -2, 3} }};
  LinearArray<int, 2, 3> e {{ LinearArray<int, 3>{{1, 2, 3}}, LinearArray<int, 3>{{5, -2, 3}} }};
  LinearArray<int, 1, 3> e1 {{ LinearArray<int, 3>{{1, 2, 3}} }};

  using linearArray::array;
  auto e2 = array(array(1, 2.2, 3), array(4.4, 5, 6));
  for(auto const l : e2) {
    for(auto const& e: l) {
      std::cout << e << ' ';
    }
    std::cout << '\n';
  }

  // std::cout << a.dot(b);
}