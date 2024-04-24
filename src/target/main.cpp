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
  for (auto idx = 5; idx < 8; ++idx) {
    auto image = data::impl::loadJPEG<PositionReader::Image>("../src/utils/positionReader/data/test/images/image.jpg"
                                                             + std::to_string(idx));
    pr.identifyObjects(image);
    std::cout << "\n\n";
  }
  return 0;
}
