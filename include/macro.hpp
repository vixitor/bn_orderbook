#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#ifndef NDEBUG
#define DEBUG(x) std::cout << "[DEBUG] " << x << "\n"
#else

#define DEBUG(x) (void(0))

#endif

inline uint64_t Parse2BnType(const std::string_view value) {
  int cnt = 0;
  uint64_t ret = 0;
  auto it = value.begin();
  while (it != value.end()) {
    if (*it == '.') break;
    ret = ret * 10 + static_cast<uint64_t>(*it) - '0';
    ++it;
  }
  if (it == value.end()) {
    throw std::runtime_error("no '.' found when parsing to uint64_t");
  }
  ++it;
  while (it != value.end()) {
    cnt++;
    ret = ret * 10 + static_cast<uint64_t>(*it) - '0';
    ++it;
  }
  if (cnt != 8) {
    throw std::runtime_error("not 8 digit after '.' not a valid uint64_t");
  }
  return ret;
}
