#include <openssl/ssl.h>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <iostream>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace ssl = asio::ssl;
namespace json = boost::json;
namespace http = beast::http;

using Tcp = asio::ip::tcp;

class SnapshotStream {
  asio::io_context ioc;
  ssl::context cxt;
  Tcp::resolver resolver;
  beast::ssl_stream<beast::tcp_stream> stream;
  const std::string host = "api.binance.com";
  std::string target;
  http::request<http::empty_body> req;

 public:
  explicit SnapshotStream(int limit = 5000);
  void GetSnapshot(json::object* snapshot);
};
