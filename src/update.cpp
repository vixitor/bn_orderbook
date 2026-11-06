#include "update.hpp"

#include "orderbook.hpp"
websocket::stream<beast::ssl_stream<beast::tcp_stream>> GetUpdateStream() {
  asio::io_context ioc;
  ssl::context cxt(ssl::context::tls_client);
  cxt.set_default_verify_paths();

  Tcp::resolver resolver(ioc);

  websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws(ioc, cxt);

  std::string host = "stream.binance.com";
  std::string port = "9443";
  std::string target = "/ws/btcusdt@depth@100ms";

  auto results = resolver.resolve(host, port);

  beast::get_lowest_layer(ws).connect(results);

  ws.next_layer().handshake(ssl::stream_base::client);

  ws.handshake(host, target);

  std::cout << "connected\n";
  return ws;
}
UpdateData::UpdateData(const std::string& text) {
  auto data = json::parse(text);
  event_type = data.at("e").as_string();
  timestamp = data.at("E").to_number<uint64_t>();
  trading_item = data.at("s").as_string();
  first_id = data.at("U").to_number<uint64_t>();
  last_id = data.at("u").to_number<uint64_t>();
  for (auto raw_level : data.at("b").as_array()) {
    auto price = Parse2BnType(raw_level.at(0).as_string());
    auto volume = Parse2BnType(raw_level.at(1).as_string());
    bids[price] = volume;
  }
  for (auto raw_level : data.at("a").as_array()) {
    auto price = Parse2BnType(raw_level.at(0).as_string());
    auto volume = Parse2BnType(raw_level.at(1).as_string());
    asks[price] = volume;
  }
}
