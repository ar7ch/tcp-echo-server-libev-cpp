#pragma once

#include <netinet/in.h>
#include <string>

namespace util {

/*
 * Set a socket as nonblocking
 * @arg socket_fd - socket to set
 */
void set_nonblocking(int socket_fd);

/*
 * Return ip-port pair from sockaddr in readable format.
 * @arg addr - sockaddr_in instamce
 * @ret pair ip-port
 */
std::pair<std::string, int> get_ip_port(sockaddr_in &addr);

} // namespace util
