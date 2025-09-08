#pragma once
#include <cstdint>
#include <string>

namespace mp {

/**
 * @brief TCP client that periodically sends metrics JSON and responds to
 *        "SNAPSHOT" requests with a snapshot JSON.
 *
 * Protocol:
 *   - Outgoing: newline-delimited JSON frames (metrics, snapshot)
 *   - Incoming: lines of text; if a line == "SNAPSHOT", send snapshot JSON
 *
 * Threading:
 *   - start() spawns a background thread; stop() joins it.
 */
class SocketClient {
public:
    SocketClient();
    ~SocketClient();

    SocketClient(const SocketClient&) = delete;
    SocketClient& operator=(const SocketClient&) = delete;

    /**
     * @brief Start the client thread. If already running, no-op.
     * @param host Server host (e.g., "127.0.0.1")
     * @param port Server port (e.g., 7777)
     */
    void start(const std::string& host = "127.0.0.1", uint16_t port = 7777);

    /**
     * @brief Stop the client thread if running. Safe to call multiple times.
     */
    void stop();

    /**
     * @brief Returns true if worker thread is running.
     */
    bool isRunning() const noexcept;

private:
    class Impl;
    Impl* impl_;
};

} // namespace mp
