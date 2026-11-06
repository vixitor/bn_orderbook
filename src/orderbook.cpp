#include "orderbook.hpp"

void OrderBook::UpdateAsks(uint64_t price, uint64_t volume) {
  asks[price] = volume;
}

void OrderBook::UpdateBids(uint64_t price, uint64_t volume) {
  bids[price] = volume;
}

std::ostream& operator<<(std::ostream& os, const OrderBook& orderbook) {
  os << "Bids: \n";
  for (auto [price, volume] : orderbook.bids) {
    os << "price: " << price << ", volume: " << volume << "\n";
  }
  os << "Asks: \n";
  for (auto [price, volume] : orderbook.asks) {
    os << "price: " << price << ", volume: " << volume << "\n";
  }
  return os;
}
