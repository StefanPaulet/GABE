//
// Created by stefan on 01.08.2023.
//

#include <Server.hpp>
#include <iostream>

using namespace gabe::server;

int main() {
  try {
    Server server;
    while (true) {
      server.getClient();
    }
  } catch (std::exception& e) {
    std::cout << e.what() << '\n';
  }
}
