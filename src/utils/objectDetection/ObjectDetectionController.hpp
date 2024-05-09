//
// Created by stefan on 4/26/24.
//

#pragma once

#include "Exceptions.hpp"
#include "types.hpp"
#include <array>
#include <iostream>
#include <regex>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace gabe::utils {

struct BoundingBox {
  Point topLeft;
  Point bottomRight;

  auto operator==(BoundingBox const& other) const -> bool = default;
};

class ObjectDetectionController {
public:
  ObjectDetectionController() = delete;
  ObjectDetectionController(ObjectDetectionController const&) = delete;
  ObjectDetectionController(ObjectDetectionController&&) = delete;

  explicit ObjectDetectionController(std::string const& scriptRootPath) noexcept(false) {
    int inPipe[2];
    int outPipe[2];

    if (pipe(inPipe) < 0) {
      throw exceptions::PipeCreationException {};
    }
    if (pipe(outPipe) < 0) {
      throw exceptions::PipeCreationException {};
    }

    _childPid = fork();
    if (_childPid == -1) {
      throw exceptions::ChildCreationException {};
    }

    if (_childPid == 0) {
      close(inPipe[0]);
      close(outPipe[1]);

      dup2(inPipe[1], STDOUT_FILENO);
      dup2(outPipe[0], STDIN_FILENO);

      execlp((scriptRootPath + "/.venv/bin/python3").c_str(), (scriptRootPath + "/.venv/bin/python3").c_str(),
             (scriptRootPath + "/objectDetection.py").c_str(), NULL);
    } else {
      _channel[0] = inPipe[0];
      _channel[1] = outPipe[1];

      std::string checkMessage = "checking communication";
      write(_channel[1], (checkMessage + "\n").c_str(), checkMessage.length() + 1);

      auto* response = new char[64];
      read(_channel[0], response, 64);
      std::string responseString {response};
      delete[] response;
      if (responseString != "response:" + checkMessage) {
        throw exceptions::ChildCommunicationException(checkMessage, responseString);
      }
    }
    takeRedundantInput();
  }

  ~ObjectDetectionController() {
    int status;

    kill(_childPid, 9);
    waitpid(_childPid, &status, 0);

    close(_channel[0]);
    close(_channel[1]);
  }

  auto analyzeImage(unsigned char* data) -> std::vector<BoundingBox> {
    static constexpr auto imageSize = 640 * 640 * 3;

    auto* response = new char[256];
    write(_channel[1], data, imageSize);
    read(_channel[0], response, 256);
    std::regex regex {R"(\[[^\[]*\d+.*\d+.*\d+.*\d+[^\]]*\])"};
    std::string string {response};
    delete[] response;

    std::vector<BoundingBox> result {};
    for (std::smatch stringMatch; std::regex_search(string, stringMatch, regex);) {
      auto match = stringMatch.str();
      std::stringstream stringstream {match.substr(1, match.size() - 1)};
      float x, y, z, t;
      stringstream >> x >> y >> z >> t;
      result.emplace_back(Point {static_cast<int>(x), static_cast<int>(y)},
                          Point {static_cast<int>(z), static_cast<int>(t)});
      string = stringMatch.suffix();
    }
    return result;
  }

private:
  auto takeRedundantInput() -> void {
    auto* response = new char[128];
    do {
      read(_channel[0], response, 128);
    } while (std::string(response).find("**finishedsetup**") == std::string::npos);
    std::cout << "Roboflow finished setup\n";
    delete[] response;
  }

  std::array<int, 2> _channel {};
  pid_t _childPid {};
};
} // namespace gabe::utils
