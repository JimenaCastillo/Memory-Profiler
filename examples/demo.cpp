#include "SocketClient.hpp"
#include "ProfilerAPI.hpp"
#include "ProfilerNew.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// Un bloque de tamaño configurable para “ver” variaciones en active_bytes
struct Blob {
    explicit Blob(size_t sz) : size(sz) { data = new char[size]; }
    ~Blob() { delete[] data; }
    size_t size;
    char*  data;
};

// Helper que crea un Blob usando MP_NEW_FT para capturar file/line/type
static Blob* make_blob(size_t sz) {
    return MP_NEW_FT(Blob, sz);
}

namespace mp { void install_callbacks_with_memorytracker(); }


int main() {

    mp::install_callbacks_with_memorytracker();
    // 1) Arranca el cliente para hablar con la GUI (127.0.0.1:7777 por defecto)
    static mp::SocketClient client;
    client.start("127.0.0.1", 7777);

    std::cout << "[demo] starting... metrics(before)="
            << mp::api::getMetricsJson() << "\n";

    std::mt19937_64 rng{123456789};
    std::uniform_int_distribution<size_t> size_dist(1<<10, 1<<15); // 1 KB .. 32 KB
    std::bernoulli_distribution del_dist(0.45);

    std::vector<Blob*> live;  // objetos en vida
    live.reserve(10'000);

    // FASE A) Warmup: sube active_bytes de forma estable
    std::cout << "[demo] WARMUP...\n";
    for (int i = 0; i < 400; ++i) {
        size_t sz = size_dist(rng);
        live.push_back(make_blob(sz));                 // allocate
        if (i % 50 == 0) {
            std::cout << "[demo] metrics=" << mp::api::getMetricsJson() << "\n";
        }
        std::this_thread::sleep_for(10ms);
    }

    // FASE B) Churn: asigna y libera para hacer “sierra” en la gráfica
    std::cout << "[demo] CHURN...\n";
    for (int i = 0; i < 800; ++i) {
        // 2 asignaciones
        live.push_back(make_blob(size_dist(rng)));
        live.push_back(make_blob(size_dist(rng)));

        // 0–2 liberaciones probabilísticas
        for (int k = 0; k < 2; ++k) {
            if (!live.empty() && del_dist(rng)) {
                delete live.back();
                live.pop_back();
            }
        }

        if (i % 100 == 0) {
            std::cout << "[demo] metrics=" << mp::api::getMetricsJson() << "\n";
        }
        std::this_thread::sleep_for(8ms);
    }

    // FASE C) Leak controlado: dejamos algunos vivos para que active_bytes no vuelva a cero
    std::cout << "[demo] CONTROLLED LEAK...\n";
    for (int i = 0; i < 200; ++i) {
        live.push_back(make_blob(size_dist(rng)));
        if (i % 40 == 0) {
            std::cout << "[demo] metrics=" << mp::api::getMetricsJson() << "\n";
        }
        std::this_thread::sleep_for(12ms);
    }

    // Libera ~70% y deja ~30% sin liberar (leak intencional para ver en snapshot)
    size_t to_free = (live.size() * 70) / 100;
    for (size_t i = 0; i < to_free; ++i) {
        delete live.back();
        live.pop_back();
    }

    std::cout << "[demo] metrics(final, with leak)=" << mp::api::getMetricsJson() << "\n";
    std::cout << "[demo] sleeping 5s — check GUI (graph + counter). You can trigger SNAPSHOT from server.\n";
    std::this_thread::sleep_for(5s);

    // Cierra el cliente (los objetos que quedaron en 'live' quedan como leak a propósito)
    client.stop();
    std::cout << "[demo] done.\n";
    return 0;
}
