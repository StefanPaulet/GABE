//
// Created by stefan on 2/8/24.
//

#include <iostream>
#include <utils/math/linearArray/LinearArray.hpp>

using namespace gabe::utils::math;
using linearArray::larray;

int main() {
  auto arr1 = LinearArray<int, 3> {{2, 4, 6}};
  auto arr2 = larray(1, 1, 1);
  auto arr3 = arr1 - arr2;
  auto arr4 = larray(larray(1, 3, 5), larray(2, 4, 6));
  auto arr5 = larray(larray(larray(1, 3, 5), larray(2, 4, 6)));
  std::cout << arr5 << '\n';
  std::cout << arr4 << '\n';
  std::cout << arr3 << '\n';
  std::cout << arr2 << '\n';
}
