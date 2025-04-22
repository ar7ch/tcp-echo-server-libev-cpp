#include "CLI/CLI.hpp"
#include <CLI/CLI.hpp>
#include <arpa/inet.h>
#include <errno.h>
#include <ev++.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace app {

namespace util {

void set_nonblocking(int socket_fd) {
  int flags = fcntl(socket_fd, F_GETFL, 0);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

static std::pair<std::string, int> get_ip_port(sockaddr_in &addr) {
  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(addr.sin_addr), ip_str, sizeof(ip_str));
  uint16_t port = ntohs(addr.sin_port);
  return {ip_str, port};
}

} // namespace util

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

void handle_sigint(ev::sig &sig, int revents) {
  spdlog::info("SIGINT received. Shutting down...");
  sig.loop.break_loop();
}

int main(int argc, char **argv) {
  CLI::App app{"TCP echo server"};
  spdlog::set_pattern("[%H:%M:%S.%e][%^%l%$] %v");
  spdlog::set_level(spdlog::level::info);

  int port;
  app.add_option("port", port, "Port number to listen on")
      ->required()
      ->check(CLI::Range(1, 65535));
  bool verbose = false;
  app.add_flag("-v,--verbose", verbose, "Enable verbose logging");

  CLI11_PARSE(app, argc, argv);

  if (verbose) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Verbose mode");
  }

  try {
    ev::default_loop loop{};
    spdlog::debug("Starting server on port {}...", port);
    // handle Ctrl+C
    ev::sig sigint(loop);
    sigint.set<handle_sigint>();
    sigint.start(SIGINT);
    // start server
    app::Server server(port, loop);
    loop.run();
  } catch (std::exception &e) {
    spdlog::error("Error occured: {}", e.what());
    return EXIT_FAILURE;
  }
  return 0;
}
