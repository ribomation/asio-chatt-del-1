// Chattserver, steg 1b: synkron med en läs-tråd per klient och en
// broadcast-tråd (den klassiska Java-designen).
//
//   läs-tråd (en per klient): read_until en rad -> push Message i kön
//   broadcast-tråd (en):      pop Message -> asio::write till alla utom avsändaren
//
// Läs-tråden läser och broadcast-tråden skriver på samma socket samtidigt.
// Det är tillåtet: Asios synkrona send/receive är trådsäkra i förhållande till
// varandra. Bara broadcast-tråden skriver, så meddelanden kan inte flätas ihop.
//
// Testa med flera: nc localhost 4711, eller client-simulator/chat-sim.py
#include "blocking_queue.hxx"
#include "clients.hxx"
#include <boost/asio.hpp>
#include <cstdio>
#include <exception>
#include <format>
#include <print>
#include <ranges>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>

struct Message {
    SessionPtr sender;  // får inte sitt eget meddelande tillbaka
    std::string text;   // komplett rad, inklusive '\n'
};

auto strip_eol(std::string_view s) -> std::string_view {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.remove_suffix(1);
    return s;
}

// Läs-tråd: en per klient. Lever tills klienten kopplar ned.
void reader(SessionPtr session, ClientList& clients, BlockingQueue<Message>& queue) {
    auto buf = std::string{};
    for (;;) {
        auto ec = boost::system::error_code{};
        auto n = asio::read_until(session->socket, asio::dynamic_buffer(buf), '\n', ec);
        if (ec) break;  // eof eller annat fel: klienten är borta, oavsett vilket

        auto line = strip_eol(std::string_view{buf}.substr(0, n));
        queue.push({session, std::format("[{}] {}\n", session->name, line)});
        buf.erase(0, n);
    }

    clients.remove(session);
    queue.push({session, std::format("*** {} kopplade ned\n", session->name)});
    std::println("nedkopplad: {} ({} kvar)", session->name, clients.size());
}

// Broadcast-tråd: en för hela servern. jthread skickar in stop_token själv.
void broadcaster(std::stop_token stop, ClientList& clients, BlockingQueue<Message>& queue) {
    while (auto msg = queue.pop(stop)) {
        auto others = clients.snapshot()
                    | std::views::filter([&](auto const& s) { return s != msg->sender; });
        for (auto const& s : others) {
            // Skrivfel ignoreras här: klientens egen läs-tråd upptäcker att den
            // är borta och plockar bort den ur listan.
            auto ec = boost::system::error_code{};
            asio::write(s->socket, asio::buffer(msg->text), ec);
        }
    }
}

int main(int argc, char** argv) try {
    auto port = static_cast<unsigned short>(argc > 1 ? std::stoi(argv[1]) : 4711);

    auto clients = ClientList{};
    auto queue = BlockingQueue<Message>{};
    auto broadcast_thread = std::jthread{broadcaster, std::ref(clients), std::ref(queue)};

    auto ioc = asio::io_context{};
    auto acceptor = tcp::acceptor{ioc, tcp::endpoint{tcp::v4(), port}};
    std::println("chattserver (tråd per klient) lyssnar på port {}", port);

    for (;;) {
        auto session = std::make_shared<Session>(acceptor.accept());
        clients.add(session);
        std::println("ansluten: {} ({} totalt)", session->name, clients.size());
        queue.push({session, std::format("*** {} anslöt\n", session->name)});

        // Läs-tråden städar efter sig själv och ingen behöver vänta in den,
        // därför detach. En jthread hade behövt sparas och joinas någonstans.
        std::thread{reader, session, std::ref(clients), std::ref(queue)}.detach();
    }
} catch (std::exception const& e) {
    std::println(stderr, "fel: {}", e.what());
    return 1;
}
