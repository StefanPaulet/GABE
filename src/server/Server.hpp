//
// Created by stefan on 8/9/23.
//

#pragma once

#include <CDS/util/JSON>
#include <engine/GameState.hpp>
#include <exceptions/Exceptions.hpp>
#include <fstream>
#include <httpMessage/HttpMessage.hpp>
#include <list>
#include <mutex>
#include <netinet/in.h>
#include <socket/Socket.hpp>
#include <thread/Thread.hpp>
#include <unistd.h>

namespace gabe::server {
class Server {
public:
  struct ThreadParam {
    GameState& state;
    int fd;
  };

  Server(GameState& state) noexcept(false) : _state {state} {
    _properties.sin_family = AF_INET;
    _properties.sin_addr.s_addr = htonl(INADDR_ANY);
    _properties.sin_port = htons(PORT);
    initialize();
  }
  auto getClient() -> void {
    sockaddr_in client_addr {};
    socklen_t client_addr_size = sizeof(client_addr);
    int fd;

    if (-1 == (fd = accept(_socketFd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_size))) {
      throw exceptions::AcceptException();
    }

    createThread(fd);
  }

private:
  auto createThread(int fd) noexcept(false) -> void {
    auto pTp = new ThreadParam {_state, fd};
    _threadList.emplace_back(&Server::threadMain, pTp);
  }
  inline auto initialize() noexcept(false) -> void {

    if (-1 == (_socketFd = socket(AF_INET, SOCK_STREAM, 0))) {
      throw exceptions::SocketCreationException();
    }

    if (int optionToggle = 1; -1 == setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &optionToggle, sizeof(int))) {
      throw exceptions::SocketOptionException();
    }

    if (-1 == bind(_socketFd, reinterpret_cast<sockaddr*>(&_properties), sizeof(sockaddr))) {
      throw exceptions::SocketBindException();
    }

    if (-1 == listen(_socketFd, 1)) {
      throw exceptions::ListenException();
    }
  }
  static auto threadMain(void* pParam) -> void* {
    static std::mutex myMutex {};
    static auto cout = std::ofstream("fisier.out");

    auto threadParam = std::unique_ptr<ThreadParam>(static_cast<ThreadParam*>(pParam));
    HttpMessage response {"Http 1.1/ 200 OK", {}, "ok"};
    Socket const socket {threadParam->fd};

    if (HttpMessage request = socket.read(); request.getBody().starts_with('{')) {
      try {
        auto lg = std::lock_guard {myMutex};
        try {
          auto jsonData = cds::json::parseJson(request.getBody());
          threadParam->state.update(jsonData);
        } catch (cds::Exception const& e) {
          cout << "Error\n";
        }
        cout << request << '\n';
        cout.flush();
        socket.write(response);
      } catch (exceptions::ConnectionTimeoutException& e) {
        log(std::format("Exception {} while receiving http requests", e.what()), OpState::FAILURE);
      }
    }

    close(threadParam->fd);

    return nullptr;
  }

  static constexpr int PORT = 3000;
  sockaddr_in _properties {};
  int _socketFd {};
  std::list<Thread> _threadList {};
  GameState& _state;
};
} // namespace gabe::server
