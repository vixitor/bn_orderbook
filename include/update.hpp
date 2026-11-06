#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

#include "orderbook.hpp"

namespace json = boost::json;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = asio::ssl;

using Tcp = asio::ip::tcp;

websocket::stream<beast::ssl_stream<beast::tcp_stream>> GetUpdateStream();

class UpdateData {
 public:
  std::string event_type;
  uint64_t timestamp;
  std::string trading_item;
  uint64_t first_id, last_id;
  std::map<uint64_t, uint64_t> bids, asks;

  explicit UpdateData(const std::string&);
};
