// En session per ansluten klient, plus den mutex-skyddade listan över dem.
#pragma once
#include <boost/asio.hpp>
#include <algorithm>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

auto to_string(tcp::endpoint const& ep) -> std::string {
    return std::format("{}:{}", ep.address().to_string(), ep.port());
}

// Ägs via shared_ptr av läs-tråden, klientlistan och tillfälligt av
// broadcast-trådens kopia. Socketen stängs i destruktorn, alltså först när
// ingen längre använder den. Därmed kan close() aldrig krocka med en skrivning.
struct Session {
    tcp::socket socket;
    std::string name;

    explicit Session(tcp::socket sock)
        : socket{std::move(sock)}, name{to_string(socket.remote_endpoint())} {}
};
using SessionPtr = std::shared_ptr<Session>;

class ClientList {
public:
    void add(SessionPtr session) {
        auto lock = std::scoped_lock{mtx};
        sessions.push_back(std::move(session));
    }

    void remove(SessionPtr const& session) {
        auto lock = std::scoped_lock{mtx};
        std::erase(sessions, session);
    }

    // Broadcast-tråden itererar över en kopia, så att låset inte hålls medan
    // asio::write blockerar.
    auto snapshot() -> std::vector<SessionPtr> {
        auto lock = std::scoped_lock{mtx};
        return sessions;
    }

    auto size() -> std::size_t {
        auto lock = std::scoped_lock{mtx};
        return sessions.size();
    }

private:
    std::vector<SessionPtr> sessions;
    std::mutex mtx;
};
