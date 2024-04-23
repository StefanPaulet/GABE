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
  pr.loadTemplates("../src/utils/positionReader/data", 2);
  pr.identifyObjects("../src/utils/positionReader/data", 1);
  return 0;
}
