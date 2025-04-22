#include <CLI/CLI.hpp>
#include <arpa/inet.h>
#include <ev++.h>
#include <netinet/in.h>
#include <server.hpp>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <unistd.h>

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
