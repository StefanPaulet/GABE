//
// Created by stefan on 5/15/24.
//

#pragma once

#include "Exceptions.hpp"
#include "engine/GameState.hpp"
#include "multithreaded/runnable/Runnable.hpp"
#include "multithreaded/synchronizer/Synchronizer.hpp"
#include <cstdio>
#include <thread>

namespace gabe {
class PositionReader : public Runnable<PositionReader> {
public:
  explicit PositionReader(GameState& gameState, std::string const& csgoRootPath) noexcept(false) :
      _gameState {gameState}, _filePath {csgoRootPath + "/console.log"}, _pIn {fopen(_filePath.c_str(), "r")} {
    if (_pIn == nullptr) {
      throw exceptions::InvalidCSRootPathException {};
    }
  }

  ~PositionReader() { fclose(_pIn); }

  auto mainLoop() -> void {
    constexpr auto setPosOffset = std::string("setpos").length();
    static char currentChar;
    static std::string currentString;

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
          synchronizer.handleSynchronization();
        }
      }
      currentString = std::string {};
    } else {
      currentString.push_back(currentChar);
    }
  }

  Synchronizer synchronizer;

private:
  std::string _filePath;
  FILE* _pIn;
  GameState& _gameState;
};
} // namespace gabe
