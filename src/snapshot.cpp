#include "snapshot.hpp"

#include <optional>
#include <stdexcept>

#include "macro.hpp"
SnapshotStream::SnapshotStream(int limit)
    : ioc(),
      cxt(ssl::context::tls_client),
      resolver(ioc),
      stream(ioc, cxt),
      target(std::format("/api/v3/depth?symbol=BTCUSDT&limit={}", limit)),
      req(http::verb::get, target, 11) {
  if (limit <= 0) {
    throw std::invalid_argument("invalid limit in snapshot");
  }
  cxt.set_default_verify_paths();
  cxt.set_verify_mode(ssl::verify_peer);
  stream.set_verify_callback(ssl::host_name_verification(host));
  beast::get_lowest_layer(stream).connect(resolver.resolve(host, "443"));
  DEBUG("tcp connected\n");
  stream.handshake(ssl::stream_base::client);
  DEBUG("tsl handshake\n");
  req.set(http::field::host, host);
  req.set(http::field::user_agent, "cpp_bn_orderbook");
}
void SnapshotStream::GetSnapshot(json::object* snapshot) {
  for (;;) {
    http::write(stream, req);
    DEBUG("send http request\n");
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);

    if (res.result() != http::status::ok) {
      std::cerr << "didn't get response\n";
      continue;
    }
    DEBUG("receive response\n");
    *snapshot = json::parse(res.body()).as_object();
    return;
  }
}
