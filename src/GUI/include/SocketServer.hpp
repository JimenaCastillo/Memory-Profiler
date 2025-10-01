#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <QObject>  // Necesario para señales Qt

namespace mp::gui {

    struct Metrics {
        std::uint64_t t_ms = 0;
        std::uint64_t active_bytes = 0;
        std::uint64_t peak_bytes   = 0;
        std::uint64_t total_allocs = 0;
        std::uint64_t active_allocs= 0;
    };

    class SocketServer : public QObject {
        Q_OBJECT

    public:
        explicit SocketServer(QObject* parent = nullptr);
        ~SocketServer();

        bool start(std::uint16_t port = 7777);
        void stop();
        bool isRunning() const noexcept;

        Metrics latest() const;
        std::vector<Metrics> seriesCopy() const;
        bool tryPopLastSnapshot(std::string& out);
        void requestSnapshot();

        signals:
            void metricsUpdated(const mp::gui::Metrics& m);


    private:
        class Impl;
        Impl* impl_;
    };

} // namespace mp::gui
