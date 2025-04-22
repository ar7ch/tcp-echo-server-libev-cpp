#pragma once

#include <arpa/inet.h>
#include <ev++.h>
#include <spdlog/spdlog.h>
#include <util.hpp>

namespace app {

class ClientConnection {
  int _fd;
  ev::io _watcher;
  std::pair<std::string, int> _ip_port;
  std::function<void()> _on_disconnect_cb;

public:
  static constexpr size_t BUFFER_SIZE = 4096;

  ClientConnection(int client_fd, ev::loop_ref loop,
                   std::pair<std::string, int> ip_port,
                   std::function<void()> on_disconnect_cb)
      : _fd(client_fd), _watcher(loop), _ip_port(std::move(ip_port)),
        _on_disconnect_cb(std::move(on_disconnect_cb)) {
    _watcher.set<ClientConnection, &ClientConnection::on_read>(this);
    _watcher.start(client_fd, ev::READ);
  }

  ~ClientConnection() {
    _watcher.stop();
    close(_fd);
  }

  int get_fd() const { return _fd; }

  std::pair<std::string, int> get_ip_port() const { return _ip_port; };

  void on_read(ev::io &watcher, int revents) {
    char buffer[BUFFER_SIZE];
    ssize_t n_bytes = ::read(_fd, buffer, sizeof(buffer));
    if (n_bytes <= 0) {
      spdlog::info("Client {}:{}: connection reset, closing...", _ip_port.first,
                   _ip_port.second);
      _on_disconnect_cb();
      return;
    }
    spdlog::debug("Client {}:{}: received {} bytes", _ip_port.first,
                  _ip_port.second, n_bytes);
    ::send(_fd, buffer, n_bytes, 0); // echo back
  }
};

} // namespace app
