// Echo-server, synkron: tar emot en klient i taget och skickar tillbaka allt
// den får. Testa med: nc localhost 7000
#include <boost/asio.hpp>
#include <array>
#include <cstdio>
#include <exception>
#include <print>
#include <string>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

auto to_string(tcp::endpoint const& ep) -> std::string {
    return std::format("{}:{}", ep.address().to_string(), ep.port());
}

// Ekar allt från en ansluten klient tills den kopplar ned.
// eof är normalt flöde, så det hanteras med error_code i stället för exception.
void echo(tcp::socket& sock) {
    auto buf = std::array<char, 1024>{};
    for (;;) {
        auto ec = boost::system::error_code{};
        auto n = sock.read_some(asio::buffer(buf), ec);
        if (ec == asio::error::eof) return;
        if (ec) throw boost::system::system_error{ec};
        asio::write(sock, asio::buffer(buf, n));
    }
}

int main(int argc, char** argv) try {
    auto port = static_cast<unsigned short>(argc > 1 ? std::stoi(argv[1]) : 7000);

    auto ioc = asio::io_context{};
    auto acceptor = tcp::acceptor{ioc, tcp::endpoint{tcp::v4(), port}};
    std::println("echo-server lyssnar på port {}", port);

    for (;;) {
        auto sock = acceptor.accept();
        auto peer = to_string(sock.remote_endpoint());
        std::println("ansluten: {}", peer);
        try {
            echo(sock);
            std::println("nedkopplad: {}", peer);
        } catch (std::exception const& e) {
            std::println("fel mot {}: {}", peer, e.what());
        }
    }
} catch (std::exception const& e) {
    std::println(stderr, "fel: {}", e.what());
    return 1;
}
