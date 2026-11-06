#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include "macro.hpp"
#include "orderbook.hpp"
#include "snapshot.hpp"
#include "update.hpp"
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = asio::ssl;

using Tcp = asio::ip::tcp;

OrderBook ParseSnapshot(const json::object& snapshot) {
  std::cout << "parsing snapshot\n";
  uint64_t last_update_id = snapshot.at("lastUpdateId").to_number<uint64_t>();
  OrderBook orderbook(last_update_id);
  auto bids = snapshot.at("bids").as_array();

  for (auto raw_level : bids) {
    auto level = raw_level.as_array();
    auto price = Parse2BnType(level.at(0).as_string());
    auto volume = Parse2BnType(level.at(1).as_string());
    orderbook.UpdateBids(price, volume);
  }
  auto asks = snapshot.at("asks").as_array();
  for (auto raw_level : asks) {
    auto level = raw_level.as_array();
    auto price = Parse2BnType(level.at(0).as_string());
    auto volume = Parse2BnType(level.at(1).as_string());
    orderbook.UpdateAsks(price, volume);
  }
  std::cout << "finished parsing snapshot\n";
  return orderbook;
}

int main() {
  json::object snapshot;
  std::queue<UpdateData> data_queue;
  std::optional<OrderBook> orderbook;
  std::mutex mxt;
  auto snapshot_thread = std::thread([&]() -> void {
    DEBUG("starting snapshot thread");
    auto stream = SnapshotStream();
    json::object snapshot;
    for (;;) {
      try {
        stream.GetSnapshot(&snapshot);
        auto new_orderbook = ParseSnapshot(snapshot);
        mxt.lock();
        if (orderbook->GetLastUpdateId() < new_orderbook.GetLastUpdateId()) {
          orderbook = std::move(new_orderbook);
          DEBUG("replace orderbook\n");
          std::cout << orderbook->GetLastUpdateId() << "\n";
        } else if (orderbook->GetLastUpdateId() ==
                   new_orderbook.GetLastUpdateId()) {
          DEBUG("match orderbook\n");
        }
        mxt.unlock();
      } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        std::cerr << "failed when getting snapshot\n";
        continue;
      }
    }
  });

  auto update_thread = std::thread([&]() {
    DEBUG("starting update thread");
    auto ws = []() {
      for (;;) {
        std::cout << "in the loop\n";
        try {
          return GetUpdateStream();
        } catch (const std::exception& e) {
          continue;
        }
      }
    }();

    for (;;) {
      beast::flat_buffer buffer;

      ws.read(buffer);

      std::string text = beast::buffers_to_string(buffer.data());
      auto updatedata = UpdateData(text);
      std::cout << "updatedata first id" << updatedata.first_id << "\n";
      std::cout << "updatedata last id" << updatedata.last_id << "\n";
      if (data_queue.empty()) {
        data_queue.push(updatedata);
      } else if (data_queue.front().last_id + 1 == updatedata.first_id) {
        data_queue.push(updatedata);
      } else {
        while (!data_queue.empty()) {
          data_queue.pop();
        }
        data_queue.push(updatedata);
      }
      mxt.lock();
      while (!data_queue.empty()) {
        auto data = data_queue.front();
        if (data.last_id <= orderbook->GetLastUpdateId()) {
          data_queue.pop();
          continue;
        } else if (data.first_id <= orderbook->GetLastUpdateId() + 1 &&
                   data.last_id > orderbook->GetLastUpdateId()) {
          std::cout << "updating orderbook\n";
          orderbook->UpdateLastUpdateId(data.last_id);
          for (auto [price, volume] : data.bids) {
            orderbook->UpdateBids(price, volume);
          }
          for (auto [price, volume] : data.asks) {
            orderbook->UpdateAsks(price, volume);
          }
        } else {
          break;
        }
      }
      mxt.unlock();
    }
  });
  snapshot_thread.join();
  update_thread.join();
  return 0;
}
