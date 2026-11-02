// Trådsäker kö med blockerande pop. Broadcast-tråden väntar i pop() tills ett
// meddelande finns, eller tills dess jthread begär stopp via stop_token.
#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <stop_token>
#include <utility>

template <typename T>
class BlockingQueue {
public:
    void push(T item) {
        {
            auto lock = std::scoped_lock{mtx};
            items.push_back(std::move(item));
        }
        cv.notify_one();
    }

    // Blockerar tills det finns något att hämta. Returnerar nullopt när stopp
    // har begärts, så att den anropande tråden kan avsluta sin loop.
    auto pop(std::stop_token stop) -> std::optional<T> {
        auto lock = std::unique_lock{mtx};
        if (!cv.wait(lock, stop, [this] { return !items.empty(); })) return std::nullopt;
        auto item = std::move(items.front());
        items.pop_front();
        return item;
    }

private:
    std::deque<T> items;
    std::mutex mtx;
    std::condition_variable_any cv;  // _any krävs för wait() med stop_token
};
