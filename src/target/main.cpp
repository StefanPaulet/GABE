//
// Created by stefan on 2/8/24.
//

#include <iostream>
#include <utils/math/linearArray/LinearArray.hpp>

using namespace gabe::utils::math;
using linearArray::larray;

int main() {
  //  std::array<int, 3> a1 {{1, 2, 3}};
  //  LinearArray<int, 3> a {{1, 2, 3}};
  //  LinearArray<int, 3> b {{5, -2, 3}};
  //  LinearMatrix<int, 2, 3> c {{a, b}};
  //  LinearArray<int, 2, 3> d {{a + a, b}};
  //
  //  LinearArray<int, 2, 3> {{LinearArray<int, 3> {{1, 2, 3}}, LinearArray<int, 3> {{5, -2, 3}}}};
  //  LinearArray<int, 1, 3> e1 {{ LinearArray<int, 3>{{1, 2, 3}} }};


  auto arr1 = LinearArray<int, 3> {{2, 4, 6}};
  auto arr2 = larray(1, 1, 1);
  auto arr3 = arr1 - arr2;
  auto arr4 = larray(larray(1, 3, 5), larray(2, 4, 6));
  //LA<int,
  using Inner = decltype(arr4);
  using Err = LinearArray<LinearArray<int, 2, 3>, 1>;
  using namespace linearArray::impl;
  //  GetSizePackOfMLA<Err>::type x = 1;
  auto arr5 = larray(larray(larray(1, 3, 5), larray(2, 4, 6)));
  std::cout << arr5 << '\n';
  std::cout << arr4 << '\n';
  std::cout << arr3 << '\n';
  std::cout << arr2 << '\n';
}
