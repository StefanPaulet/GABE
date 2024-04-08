//
// Created by stefan on 2/8/24.
//
#include <neural_net/ObjectRecognition.hpp>
#include <sys/resource.h>
#include <utils/data/dataLoader/DataLoader.hpp>
#include <utils/math/linearArray/LinearArray.hpp>

#include <chrono>

namespace {
using namespace gabe::nn;
using namespace gabe::utils::data;
using namespace gabe::utils::math;
using namespace gabe::utils::math::linearArray;
} // namespace

int main() {
  rlimit rl;
  getrlimit(RLIMIT_STACK, &rl);
  rl.rlim_cur = 31 * 1024 * 1024 * 8;
  setrlimit(RLIMIT_STACK, &rl);
    auto smth = loadCS2Images<LinearArray<double, 3, 640, 640>>("../data/yolo/cs2/train", 5);
  ObjectDetection::InitializedObjectRecongnitionNet nn {};
//  std::cout << sizeof(nn) << '\n';

  auto start = std::chrono::system_clock::now();
    auto obj = nn.feedForward(smth.data().data()[0].data);
  auto end = std::chrono::system_clock::now();
std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << '\n';
    std::cout << obj << '\n';
  return 0;
}