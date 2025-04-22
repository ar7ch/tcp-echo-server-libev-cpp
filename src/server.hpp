#pragma once

#include <arpa/inet.h>
#include <client_connection.hpp>
#include <errno.h>
#include <ev++.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <util.hpp>

namespace app {

class Server {
private:
  int _port;
  ev::loop_ref _loop;
  ev::io _accept_watcher;
  int _listen_socket_fd = -1;
  std::unordered_map<int, std::unique_ptr<ClientConnection>> _fd_to_clients;

  Server(const Server &other) = delete;
  Server(const Server &&other) = delete;
  Server &operator=(const Server &other) = delete;
  Server &operator=(Server &&other) = delete;

  void _remove_client(int client_fd) {
    auto ip_port = _fd_to_clients.at(client_fd)->get_ip_port();
    spdlog::debug("Removing client {}:{}", ip_port.first, ip_port.second);
    _fd_to_clients.erase(client_fd);
  }
  void _add_client(std::unique_ptr<ClientConnection> client) {
    auto ip_port = client->get_ip_port();
    spdlog::debug("Adding client {}:{}", ip_port.first, ip_port.second);
    _fd_to_clients.insert({client->get_fd(), std::move(client)});
  }

public:
  Server(int port, ev::loop_ref &loop)
      : _port{port}, _loop{loop}, _accept_watcher{_loop} {
    _listen_socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_listen_socket_fd < 0) {
      throw std::runtime_error(
          fmt::format("socket() failed: {}", strerror(errno)));
    }

    int opt = 1;
    setsockopt(_listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    util::set_nonblocking(_listen_socket_fd);
    spdlog::debug("Socket created ok");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(_listen_socket_fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
      throw std::runtime_error(
          fmt::format("bind() failed: {}", strerror(errno)));
    }

    if (::listen(_listen_socket_fd, SOMAXCONN) != 0) {
      throw std::runtime_error(
          fmt::format("listen() failed: {}", strerror(errno)));
    }
    spdlog::debug("Bind to port {} ok", port);

    _accept_watcher.set<Server, &Server::on_accept>(this);
    _accept_watcher.start(_listen_socket_fd, ev::READ);
    spdlog::info("Server is listening on port {}", port);
  }

  ~Server() {
    _accept_watcher.stop();
    ::close(_listen_socket_fd);
  }

  void on_accept(ev::io &watcher, int revents) {
    sockaddr_in client_addr{};
    socklen_t client_addr_len;
    int client_fd =
        accept(_listen_socket_fd, (sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
      throw std::runtime_error(
          fmt::format("accept() failed: {}", strerror(errno)));
    }
    util::set_nonblocking(client_fd);
    auto ip_port = util::get_ip_port(client_addr);
    spdlog::info("Client {}:{} connected", ip_port.first, ip_port.second);
    auto remove_client_cb = [this, client_fd] { _remove_client(client_fd); };
    _add_client(std::make_unique<ClientConnection>(client_fd, _loop, ip_port,
                                                   remove_client_cb));
  }
};

} // namespace app
