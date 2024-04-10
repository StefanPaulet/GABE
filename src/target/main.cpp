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
  auto smth = loadCS2Images<LinearArray<double, 3, 640, 640>>("../data/yolo/cs2/train", 5);
  ObjectDetection::InitializedObjectRecongnitionNet nn {};
  auto start = std::chrono::system_clock::now();
  auto target = LinearArray<double, 7 * 7 * 6, 1> {};
  std::ignore = nn.backPropagate(smth.data().data()[0].data, target, 0.005);
  auto end = std::chrono::system_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main() {
  constexpr auto v = sizeof(ObjectDetection::InitializedObjectRecongnitionNet);
  rlimit rl;
  getrlimit(RLIMIT_STACK, &rl);
  rl.rlim_cur = rl.rlim_max;
  setrlimit(RLIMIT_STACK, &rl);
  f();
  return 0;
}