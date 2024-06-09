//
// Created by stefan on 01.08.2023.
//

#include <Server.hpp>
#include <engine/GameState.hpp>
#include <iostream>

using namespace gabe::server;

int main() {
  try {
    gabe::GameState state;
    Server server {state};
    while (true) {
      server.getClient();
    }
  } catch (std::exception& e) {
    std::cout << e.what() << '\n';
  }
}
