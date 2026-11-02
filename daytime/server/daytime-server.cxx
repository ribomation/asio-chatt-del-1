// Daytime-server, synkron (RFC 867): för varje klient som ansluter skrivs
// aktuell tid som en textrad, och sedan stängs anslutningen.
#include <boost/asio.hpp>
#include <chrono>
#include <cstdio>
#include <exception>
#include <format>
#include <print>
#include <string>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

auto to_string(tcp::endpoint const& ep) -> std::string {
    return std::format("{}:{}", ep.address().to_string(), ep.port());
}

auto now_as_text() -> std::string {
    using namespace std::chrono;
    auto now = zoned_time{current_zone(), floor<seconds>(system_clock::now())};
    return std::format("{:%Y-%m-%d %H:%M:%S %Z}\n", now);
}

int main(int argc, char** argv) try {
    auto port = static_cast<unsigned short>(argc > 1 ? std::stoi(argv[1]) : 1313);

    auto ioc = asio::io_context{};
    auto acceptor = tcp::acceptor{ioc, tcp::endpoint{tcp::v4(), port}};
    std::println("daytime-server lyssnar på port {}", port);

    for (;;) {
        auto sock = acceptor.accept();
        auto msg = now_as_text();

        auto ec = boost::system::error_code{};
        asio::write(sock, asio::buffer(msg), ec);
        if (ec) {
            std::println("fel mot {}: {}", to_string(sock.remote_endpoint()), ec.message());
        } else {
            std::print("skickade till {}: {}", to_string(sock.remote_endpoint()), msg);
        }
        // sock stängs här, när den går ur scope. Klienten ser det som eof.
    }
} catch (std::exception const& e) {
    std::println(stderr, "fel: {}", e.what());
    return 1;
}
