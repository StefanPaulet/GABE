//
// Created by stefan on 8/9/23.
//

#pragma once

#include <exceptions/Exceptions.hpp>
#include <pthread.h>

namespace gabe::server {
class Thread {
public:
  using ThreadFunctionType = void* (*) (void*);

  template <typename ThreadFuncParamType>
  Thread(ThreadFunctionType threadFunction, ThreadFuncParamType* parameter) noexcept(false) {
    if (0 > pthread_create(&_id, nullptr, threadFunction, reinterpret_cast<void*>(parameter))) {
      throw exception::ThreadCreationException();
    }
  }

private:
  pthread_t _id {};
};
} // namespace gabe::server
