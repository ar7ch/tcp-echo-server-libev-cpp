#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string>
#include <utility>

namespace util {

void set_nonblocking(int socket_fd) {
  int flags = fcntl(socket_fd, F_GETFL, 0);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

std::pair<std::string, int> get_ip_port(sockaddr_in &addr) {
  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(addr.sin_addr), ip_str, sizeof(ip_str));
  uint16_t port = ntohs(addr.sin_port);
  return {ip_str, port};
}

} // namespace util
