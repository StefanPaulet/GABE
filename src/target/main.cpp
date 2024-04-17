//
// Created by stefan on 2/8/24.
//
#include <neural_net/ObjectRecognition.hpp>
#include <sys/resource.h>
#include <utils/data/dataLoader/DataLoader.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

namespace {
using namespace gabe::nn;
using namespace gabe::utils::data;
using namespace gabe::utils::math;
using namespace gabe::utils::math::linearArray;
} // namespace

void f() {
  auto dataSet = loadCS2Images<LinearArray<double, 3, 640, 640>>("../data/yolo/cs2/train");
  dataSet.normalize();
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(dataSet.data().begin(), dataSet.data().end(), g);

  ObjectDetection::InitializedObjectRecongnitionNet nn {};
  nn.deserialize("deepObjectDetectionMarker.nn");
  auto start = std::chrono::system_clock::now();
  auto clipper = [](double value) {
    if (constexpr auto precision = 0.00001; std::abs(value) < precision && std::abs(value) > .0) {
      return value < .0 ? -precision : precision;
    }
    if (constexpr auto precision = 2.0; std::abs(value) > precision) {
      return value < .0 ? -precision : precision;
    }
    return value;
  };
  nn.yoloBackPropagateWithSerialization(30, 0.0001, dataSet, "deepObjectDetection.nn", clipper);
  nn.serialize("deepObjectDetection.nn");
  nn.yoloBackPropagateWithSerialization(75, 0.001, dataSet, "deepObjectDetection2.nn", clipper);
  nn.serialize("deepObjectDetection2.nn");
  nn.yoloBackPropagateWithSerialization(30, 0.0001, dataSet, "deepObjectDetection3.nn", clipper);
  nn.serialize("deepObjectDetection3.nn");
  nn.yoloBackPropagateWithSerialization(30, 0.00001, dataSet, "deepObjectDetection4.nn", clipper);
  nn.serialize("deepObjectDetection4.nn");
  std::cout << nn.weights<0>() << '\n';
  auto end = std::chrono::system_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << '\n';
}

void g() {
  auto dataSet = loadCS2Images<LinearArray<double, 3, 640, 640>>("../data/yolo/cs2/test");
  dataSet.normalize();
  ObjectDetection::ObjectRecongnitionNet nn {};
  //  nn.deserialize("deepObjectDetectionMarker.nn");
  nn.deserialize("deepObjectDetection.nn1025");
  auto clipper = [](double value) {
    if (constexpr auto precision = 0.00001; std::abs(value) < precision && std::abs(value) > .0) {
      return value < .0 ? -precision : precision;
    }
    if (constexpr auto precision = 2.0; std::abs(value) > precision) {
      return value < .0 ? -precision : precision;
    }
    return value;
  };
  auto rez = nn.feedForward(dataSet.data()[52].data);
  std::cout << "Expected:\n";
  for (auto const& t : dataSet.data()[52].labels) {
    std::cout << t << '\n';
  }
  for (auto idx = 0; idx < 7 * 7; ++idx) {
    std::cout << "Grid cell " << idx / 7 << " " << idx % 7 << ":\n";
    auto currGrid = idx * 5 * 2;
    for (auto bIdx = 0; bIdx < 2; ++bIdx) {
      std::cout << rez[currGrid + bIdx * 5][0] << " ";
      std::cout << rez[currGrid + bIdx * 5 + 1][0] << " ";
      std::cout << rez[currGrid + bIdx * 5 + 2][0] << " ";
      std::cout << rez[currGrid + bIdx * 5 + 3][0] << " ";
      std::cout << rez[currGrid + bIdx * 5 + 4][0] << " ";
      std::cout << '\n';
    }
    std::cout << '\n';
  }
}

int main() {
  constexpr auto v = sizeof(ObjectDetection::InitializedObjectRecongnitionNet);
  rlimit rl;
  getrlimit(RLIMIT_STACK, &rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_STACK, &rl);
  f();
  //  g();
  //  h();
  return 0;
}
