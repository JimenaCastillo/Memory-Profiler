#include "SocketServer.hpp"

#include <chrono>
#include <cstring>
#include <thread>
#include <string>
#include <vector>
#include <iostream>   // logging a stdout
#include <algorithm>
#include <cassert>

// POSIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <fcntl.h>

namespace mp::gui {

// ------------------------- Helpers -------------------------

static int createListenSocket(uint16_t port) {
    int sock = ::socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
        // fallback a IPv4
        sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return -1;
    }

    int yes = 1;
    ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // Intento bind dual-stack (IPv6 con v6only=0); si falla, bind IPv4
    bool bound = false;
    {
        // IPv6
        sockaddr_in6 addr6{};
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr   = in6addr_any;
        addr6.sin6_port   = htons(port);

        int v6only = 0;
        ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));

        if (::bind(sock, (sockaddr*)&addr6, sizeof(addr6)) == 0) {
            bound = true;
        }
    }

    if (!bound) {
        ::close(sock);
        sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return -1;

        int yes2 = 1;
        ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes2, sizeof(yes2));

        sockaddr_in addr4{};
        addr4.sin_family      = AF_INET;
        addr4.sin_addr.s_addr = htonl(INADDR_ANY);
        addr4.sin_port        = htons(port);

        if (::bind(sock, (sockaddr*)&addr4, sizeof(addr4)) != 0) {
            ::close(sock);
            return -1;
        }
    }

    if (::listen(sock, 1) != 0) {
        ::close(sock);
        return -1;
    }
    return sock;
}

static uint64_t extractNumber(const std::string& json, const char* key) {
    // naive parsing: busca "key":<numero>
    std::string pat = std::string("\"") + key + "\":";
    auto pos = json.find(pat);
    if (pos == std::string::npos) return 0;
    pos += pat.size();
    while (pos < json.size() && (json[pos] == ' ')) ++pos;

    uint64_t value = 0;
    while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
        value = value * 10 + (json[pos] - '0');
        ++pos;
    }
    return value;
}

static std::string trimCopy(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && (s[i]==' '||s[i]=='\t'||s[i]=='\r'||s[i]=='\n')) ++i;
    while (j > i && (s[j-1]==' '||s[j-1]=='\t'||s[j-1]=='\r'||s[j-1]=='\n')) --j;
    return s.substr(i, j - i);
}

// ------------------------- Impl -------------------------

class SocketServer::Impl {
public:
    Impl() = default;
    ~Impl() { stop(); }

    bool start(uint16_t port) {
        std::lock_guard<std::mutex> lk(m_);
        if (running_) return true;
        port_ = port;
        running_ = true;
        worker_ = std::thread(&Impl::runLoop, this);
        return true;
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (!running_) return;
            running_ = false;
        }
        if (worker_.joinable()) worker_.join();
        closeFd(client_fd_);
        closeFd(listen_fd_);
    }

    bool isRunning() const noexcept { return running_; }

    Metrics latest() const {
        std::lock_guard<std::mutex> lk(data_m_);
        return last_;
    }

    std::vector<Metrics> seriesCopy() const {
        std::lock_guard<std::mutex> lk(data_m_);
        return series_;
    }

    bool tryPopLastSnapshot(std::string& out) {
        std::lock_guard<std::mutex> lk(data_m_);
        if (last_snapshot_.empty()) return false;
        out = std::move(last_snapshot_);
        last_snapshot_.clear();
        return true;
    }

    void requestSnapshot() {
        snapshot_flag_.store(true, std::memory_order_release);
    }

private:
    static void closeFd(int& fd) {
        if (fd >= 0) { ::close(fd); fd = -1; }
    }

    void runLoop() {
        using clock = std::chrono::steady_clock;
        start_tp_ = clock::now();

        // Crear socket de escucha
        listen_fd_ = createListenSocket(port_);
        if (listen_fd_ < 0) {
            std::cerr << "[SocketServer] Failed to bind/listen on port " << port_ << "\n";
            running_ = false;
            return;
        }
        std::cout << "[SocketServer] Listening on port " << port_ << " ...\n";

        while (running_) {
            if (client_fd_ < 0) {
                // Esperar cliente (con timeout breve para poder salir)
                struct pollfd pfd { listen_fd_, POLLIN, 0 };
                int pr = ::poll(&pfd, 1, 250);
                if (pr > 0 && (pfd.revents & POLLIN)) {
                    client_fd_ = ::accept(listen_fd_, nullptr, nullptr);
                    if (client_fd_ >= 0) {
                        std::cout << "[SocketServer] Client connected.\n";
                        rx_.clear();
                        // En cuanto conecte, pedimos métricas (el cliente las mandará periódicamente solo)
                    }
                }
                continue; // loop
            }

            // Cliente conectado: recibir líneas y, si se solicita, emitir "SNAPSHOT\n"
            struct pollfd pfd { client_fd_, POLLIN, 0 };
            int pr = ::poll(&pfd, 1, 100);
            if (pr < 0) {
                // error: reiniciar
                std::cerr << "[SocketServer] poll error; closing client.\n";
                closeFd(client_fd_);
                continue;
            }

            if (pr > 0 && (pfd.revents & POLLIN)) {
                char buf[4096];
                ssize_t n = ::recv(client_fd_, buf, sizeof(buf), 0);
                if (n <= 0) {
                    std::cout << "[SocketServer] Client disconnected.\n";
                    closeFd(client_fd_);
                } else {
                    rx_.append(buf, static_cast<size_t>(n));
                    // Procesar por líneas
                    for (;;) {
                        auto pos = rx_.find('\n');
                        if (pos == std::string::npos) break;
                        std::string line = trimCopy(rx_.substr(0, pos));
                        rx_.erase(0, pos + 1);
                        handleLine(line);
                    }
                }
            }

            // ¿Se pidió snapshot desde la GUI?
            if (snapshot_flag_.exchange(false, std::memory_order_acq_rel)) {
                sendLine("SNAPSHOT\n"); // el cliente responderá con el JSON del snapshot
            }
        }

        closeFd(client_fd_);
        closeFd(listen_fd_);
    }

    void handleLine(const std::string& line) {
        if (line.empty()) return;

        if (line == "SNAPSHOT") {
            // Cliente pidió snapshot; para mantener compatibilidad con el cliente que
            // responde a SNAPSHOT, hacemos eco: reenviamos SNAPSHOT para que él nos devuelva el JSON.
            sendLine("SNAPSHOT\n");
            return;
        }

        if (!line.empty() && line[0] == '{') {
            // JSON: puede ser métricas o snapshot
            if (line.find("\"active_bytes\"") != std::string::npos) {
                // Métricas
                Metrics m;
                m.active_bytes  = extractNumber(line, "active_bytes");
                m.peak_bytes    = extractNumber(line, "peak_bytes");
                m.total_allocs  = extractNumber(line, "total_allocs");
                m.active_allocs = extractNumber(line, "active_allocs");
                auto now = std::chrono::steady_clock::now();
                m.t_ms = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - start_tp_).count();

                {
                    std::lock_guard<std::mutex> lk(data_m_);
                    last_ = m;
                    series_.push_back(m);
                    if (series_.size() > 2000) { // evita crecimiento infinito
                        series_.erase(series_.begin(), series_.begin() + 1000);
                    }
                }

                // Log básico
                std::cout << "[METRICS] active=" << m.active_bytes
                          << "B, peak=" << m.peak_bytes
                          << "B, total_allocs=" << m.total_allocs
                          << ", active_allocs=" << m.active_allocs << "\n";
            } else {
                // Asumimos snapshot (o algún otro JSON)
                std::lock_guard<std::mutex> lk(data_m_);
                last_snapshot_ = line;
                std::cout << "[SNAPSHOT] JSON received (" << line.size() << " bytes)\n";
            }
        } else {
            // Otra línea informativa
            std::cout << "[INFO] " << line << "\n";
        }
    }

    void sendLine(const std::string& s) {
        if (client_fd_ < 0) return;
        size_t sent = 0;
        while (sent < s.size()) {
            ssize_t n = ::send(client_fd_, s.data() + sent, s.size() - sent, MSG_NOSIGNAL);
            if (n < 0) {
                if (errno == EINTR) continue;
                std::cerr << "[SocketServer] send error; closing client.\n";
                closeFd(client_fd_);
                break;
            }
            sent += static_cast<size_t>(n);
        }
    }

private:
    // estado red
    int listen_fd_ { -1 };
    int client_fd_ { -1 };
    uint16_t port_ { 7777 };

    // control
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::mutex m_;

    // buffers y datos
    std::string rx_;

    // datos para GUI
    mutable std::mutex data_m_;
    Metrics last_{};
    std::vector<Metrics> series_;
    std::string last_snapshot_;

    // snapshot on-demand
    std::atomic<bool> snapshot_flag_{false};

    // tiempo base
    std::chrono::steady_clock::time_point start_tp_;
};

// ------------------------- API thin wrapper -------------------------

SocketServer::SocketServer() : impl_(new Impl()) {}
SocketServer::~SocketServer() { if (impl_) { impl_->stop(); delete impl_; } }

bool SocketServer::start(uint16_t port)  { return impl_->start(port); }
void SocketServer::stop()                { impl_->stop(); }
bool SocketServer::isRunning() const noexcept { return impl_->isRunning(); }

Metrics SocketServer::latest() const                     { return impl_->latest(); }
std::vector<Metrics> SocketServer::seriesCopy() const    { return impl_->seriesCopy(); }
bool SocketServer::tryPopLastSnapshot(std::string& out)  { return impl_->tryPopLastSnapshot(out); }
void SocketServer::requestSnapshot()                     { impl_->requestSnapshot(); }

} // namespace mp::gui
