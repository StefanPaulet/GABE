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
  ObjectDetection::InitializedObjectRecongnitionNet nn {};
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
  for (auto idx = 0; idx < 250; ++idx) {
    nn.backPropagate(dataSet.data()[0].data, dataSet.data()[0].labels, 0.0001, clipper);
  }
  nn.serialize("objectDetection.nn");
  std::cout << nn.weights<0>() << '\n';
  auto end = std::chrono::system_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << '\n';
}

void h() {
  auto dataSet = loadCS2Images<LinearArray<double, 3, 640, 640>>("../data/yolo/cs2/train");
  dataSet.normalize();
  ObjectDetection::ObjectRecongnitionNet nn {};
  nn.deserialize("objectDetection.nn");
  auto rez = nn.feedForward(dataSet.data()[0].data);
  std::cout << dataSet.data()[0].labels[0] << '\n';

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

void g() {
  auto dataSet = loadCS2Images<LinearArray<double, 3, 640, 640>>("../data/yolo/cs2/test");
  dataSet.normalize();
  ObjectDetection::ObjectRecongnitionNet nn {};
  nn.deserialize("objectDetection.nn");

  std::cout << nn.feedForward(dataSet.data()[30].data) << '\n';
  //  auto rez = nn.feedForward(dataSet.data()[30].data);
  //  std::cout << "Expected: ";
  //  for (auto const& e : dataSet.data()[30].labels) {
  //    std::cout << e << '\n';
  //  }
  //  for (auto idx = 0; idx < 7 * 7; ++idx) {
  //    std::cout << "Grid cell " << idx / 7 << " " << idx % 7 << ":\n";
  //    auto currGrid = idx * 5 * 2;
  //    for (auto bIdx = 0; bIdx < 2; ++bIdx) {
  //      std::cout << rez[currGrid + bIdx * 5][0] << " ";
  //      std::cout << rez[currGrid + bIdx * 5 + 1][0] << " ";
  //      std::cout << rez[currGrid + bIdx * 5 + 2][0] << " ";
  //      std::cout << rez[currGrid + bIdx * 5 + 3][0] << " ";
  //      std::cout << rez[currGrid + bIdx * 5 + 4][0] << " ";
  //      std::cout << '\n';
  //    }
  //    std::cout << '\n';
  //  }
}

int main() {
  constexpr auto v = sizeof(ObjectDetection::InitializedObjectRecongnitionNet);
  rlimit rl;
  getrlimit(RLIMIT_STACK, &rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_STACK, &rl);
  //  f();
  //  g();
  h();
  return 0;
}
