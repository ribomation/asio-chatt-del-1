// Daytime-klient, synkron: slår upp värdnamnet, ansluter till första endpoint
// som fungerar och läser svaret tills servern stänger (eof).
#include <boost/asio.hpp>
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

int main(int argc, char** argv) try {
    auto host = std::string{argc > 1 ? argv[1] : "localhost"};
    auto port = std::string{argc > 2 ? argv[2] : "1313"};

    auto ioc = asio::io_context{};

    // resolve() ger en lista av endpoints (t.ex. både IPv4 och IPv6 för localhost),
    // och asio::connect() provar dem i tur och ordning tills en lyckas.
    auto resolver = tcp::resolver{ioc};
    auto endpoints = resolver.resolve(host, port);
    auto sock = tcp::socket{ioc};
    auto ep = asio::connect(sock, endpoints);
    std::println("ansluten till {}", to_string(ep));

    // Läs tills servern stänger. eof är det förväntade slutet, inte ett fel.
    auto reply = std::string{};
    auto ec = boost::system::error_code{};
    asio::read(sock, asio::dynamic_buffer(reply), ec);
    if (ec && ec != asio::error::eof) throw boost::system::system_error{ec};

    std::print("svar: {}", reply);
} catch (std::exception const& e) {
    std::println(stderr, "fel: {}", e.what());
    return 1;
}
