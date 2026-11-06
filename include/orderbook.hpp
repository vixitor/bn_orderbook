#pragma once
#include <cstdint>
#include <iostream>
#include <map>

#include "macro.hpp"
class OrderBook {
  std::map<uint64_t, uint64_t> bids, asks;
  uint64_t last_update_id;

 public:
  explicit OrderBook(uint64_t last_update_id)
      : last_update_id(last_update_id) {}
  void UpdateBids(uint64_t price, uint64_t volume);
  void UpdateAsks(uint64_t price, uint64_t volume);
  friend std::ostream& operator<<(std::ostream& os, const OrderBook&);
  uint64_t GetLastUpdateId() { return last_update_id; }
  void UpdateLastUpdateId(uint64_t id) { last_update_id = id; }
};
