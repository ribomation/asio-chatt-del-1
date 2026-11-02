// Chattserver, steg 1a: synkron och en klient i taget.
//
// Servern läser rader och skickar tillbaka varje rad med prefixet "[du] ", så
// att det syns att den arbetar. Ansluter en andra klient händer ingenting för
// den: den ligger i accept-kön tills den första kopplar ned. Det är poängen.
//
// Testa med: nc localhost 4711
#include <boost/asio.hpp>
#include <cstdio>
#include <exception>
#include <format>
#include <print>
#include <string>
#include <string_view>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

auto to_string(tcp::endpoint const& ep) -> std::string {
    return std::format("{}:{}", ep.address().to_string(), ep.port());
}

// Tar bort radslut ("\n" eller "\r\n") för loggutskriften.
auto strip_eol(std::string_view s) -> std::string_view {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.remove_suffix(1);
    return s;
}

// Pratar med en klient tills den kopplar ned (eof).
void chat(tcp::socket& sock, std::string const& peer) {
    auto buf = std::string{};
    for (;;) {
        auto ec = boost::system::error_code{};
        auto n = asio::read_until(sock, asio::dynamic_buffer(buf), '\n', ec);
        if (ec == asio::error::eof) return;
        if (ec) throw boost::system::system_error{ec};

        // read_until kan ha läst förbi radslutet. Raden är de första n byten,
        // resten tillhör nästa rad och ska ligga kvar i buf.
        auto line = std::string_view{buf}.substr(0, n);
        std::println("{}: {}", peer, strip_eol(line));

        asio::write(sock, asio::buffer(std::format("[du] {}", line)));
        buf.erase(0, n);
    }
}

int main(int argc, char** argv) try {
    auto port = static_cast<unsigned short>(argc > 1 ? std::stoi(argv[1]) : 4711);

    auto ioc = asio::io_context{};
    auto acceptor = tcp::acceptor{ioc, tcp::endpoint{tcp::v4(), port}};
    std::println("chattserver (en klient i taget) lyssnar på port {}", port);

    for (;;) {
        auto sock = acceptor.accept();
        auto peer = to_string(sock.remote_endpoint());
        std::println("ansluten: {}", peer);
        try {
            chat(sock, peer);
            std::println("nedkopplad: {}", peer);
        } catch (std::exception const& e) {
            std::println("fel mot {}: {}", peer, e.what());
        }
    }
} catch (std::exception const& e) {
    std::println(stderr, "fel: {}", e.what());
    return 1;
}
