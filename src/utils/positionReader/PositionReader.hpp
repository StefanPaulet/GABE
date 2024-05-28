//
// Created by stefan on 5/15/24.
//

#pragma once

#include "Exceptions.hpp"
#include "engine/GameState.hpp"
#include "synchronizer/Synchronizer.hpp"
#include <cstdio>
#include <thread>

namespace gabe::utils {
class PositionReader {
public:
  explicit PositionReader(GameState& gameState, std::string const& csgoRootPath) noexcept(false) :
      _gameState {gameState}, _filePath {csgoRootPath + "/console.log"}, _pIn {fopen(_filePath.c_str(), "r")} {
    if (_pIn == nullptr) {
      throw exceptions::InvalidCSRootPathException {};
    }
  }

  ~PositionReader() { fclose(_pIn); }

  auto run() -> std::jthread {
    auto mainLoop = [this]() {
      constexpr auto setPosOffset = std::string("setpos").length();
      char currentChar;
      std::string currentString;
      while (!_stopped) {
        if (feof(_pIn)) {
          auto lastPos = ftell(_pIn);
          fclose(_pIn);
          _pIn = fopen(_filePath.c_str(), "r");
          fseek(_pIn, lastPos, SEEK_SET);
        }
        fread(&currentChar, 1, 1, _pIn);
        if (currentChar == '\n') {
          if (auto pos = currentString.find("setpos"); pos != std::string::npos) {
            auto stringStream = std::stringstream {currentString.substr(pos + setPosOffset)};
            Position position;
            Orientation orientation;
            std::string angleDelimiter;
            stringStream >> position.x >> position.y >> position.z;
            stringStream >> angleDelimiter >> orientation.x >> orientation.y >> orientation.z;
            _gameState.set(GameState::Properties::POSITION, position);
            _gameState.set(GameState::Properties::ORIENTATION, orientation);
            if (synchronizer.synchronizationRequired()) {
              log("Synchronization sent", OpState::INFO);
              synchronizer.handleSynchronization();
            }
          }
          currentString = std::string {};
        } else {
          currentString.push_back(currentChar);
        }
      }
    };
    return std::jthread {mainLoop};
  }

  auto stop() -> void { _stopped = true; }

  Synchronizer synchronizer;

private:
  std::string _filePath;
  FILE* _pIn;
  bool volatile _stopped {false};
  GameState& _gameState;
};
} // namespace gabe::utils
