//
// Created by stefan on 2/8/24.
//
#include <sys/resource.h>
#include <utils/positionReader/PositionReader.hpp>

namespace {
using namespace gabe::nn;
using namespace gabe::utils;
using namespace gabe::utils::math;
using namespace gabe::utils::math::linearArray;
} // namespace

int main() {
  PositionReader pr {};
  pr.loadTemplates("../src/utils/positionReader/data/train");
  for (auto idx = 0; idx < 8; ++idx) {
    auto image = data::impl::loadJPEG<PositionReader::Image>("../src/utils/positionReader/data/test/images/image.jpg"
                                                             + std::to_string(idx));
    auto rez = pr.identifyObjects(image);
    std::cout << rez[0] << " " << rez[1] << " " << rez[2] << '\n';
    std::cout << "\n\n";
  }
  return 0;
}
