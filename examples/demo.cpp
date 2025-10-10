#include "SocketClient.hpp"
#include "ProfilerAPI.hpp"
#include "ProfilerNew.hpp"
#include "CallbacksRegistration.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// Un bloque de tamaño configurable para "ver" variaciones en active_bytes
struct Blob {
    explicit Blob(size_t sz) : size(sz) { data = new char[size]; }
    ~Blob() { delete[] data; }
    size_t size;
    char*  data;
};

// ============ AÑADIR ESTAS ESTRUCTURAS NUEVAS ============
struct TestData {
    int* values;
    explicit TestData(int n) : values(new int[n]) {}
    ~TestData() { delete[] values; }
};

struct ComplexObject {
    std::string name;
    std::vector<int> data;
    double* matrix;

    explicit ComplexObject(const std::string& n, size_t size)
        : name(n), matrix(new double[size * size]) {
        data.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            data.push_back(static_cast<int>(i));
        }
    }

    ~ComplexObject() { delete[] matrix; }
};
// ========================================================

// Helper que crea un Blob usando MP_NEW_FT para capturar file/line/type
static Blob* make_blob(size_t sz) {
    return MP_NEW_FT(Blob, sz);
}

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

    // ============ FASE INICIAL: Asignaciones de prueba ============
    std::cout << "[demo] FASE DE PRUEBA: Creando objetos diversos...\n";

    // Crear varios TestData para verificar el rastreo
    auto test1 = MP_NEW_FT(TestData, 100);   // Esta línea aparecerá en la GUI
    auto test2 = MP_NEW_FT(TestData, 200);   // Esta línea aparecerá en la GUI
    auto test3 = MP_NEW_FT(TestData, 300);   // Esta línea aparecerá en la GUI

    std::cout << "[demo] TestData creados. Esperando 2s...\n";
    std::this_thread::sleep_for(2s);

    // Crear objetos complejos
    auto complex1 = MP_NEW_FT(ComplexObject, "Objeto-A", 50);  // Esta línea aparecerá
    auto complex2 = MP_NEW_FT(ComplexObject, "Objeto-B", 75);  // Esta línea aparecerá

    std::cout << "[demo] Objetos complejos creados. Esperando 2s...\n";
    std::this_thread::sleep_for(2s);

    // Liberar algunos para ver cómo cambia
    delete test1;
    delete complex1;

    std::cout << "[demo] Algunos objetos liberados. Esperando 2s...\n";
    std::cout << "[demo] metrics=" << mp::api::getMetricsJson() << "\n";
    std::this_thread::sleep_for(2s);
    // ================================================================

    // FASE A) Warmup: sube active_bytes de forma estable
    std::cout << "[demo] WARMUP...\n";
    for (int i = 0; i < 400; ++i) {
        size_t sz = size_dist(rng);
        live.push_back(make_blob(sz));                 // allocate (línea 24 original)
        if (i % 50 == 0) {
            std::cout << "[demo] metrics=" << mp::api::getMetricsJson() << "\n";
        }
        std::this_thread::sleep_for(10ms);
    }

    // FASE B) Churn: asigna y libera para hacer "sierra" en la gráfica
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
    std::cout << "[demo] ⚠️  OBJETOS PENDIENTES (para verificar GUI):\n";
    std::cout << "[demo]    - test2 (TestData, 200 ints)\n";
    std::cout << "[demo]    - test3 (TestData, 300 ints)\n";
    std::cout << "[demo]    - complex2 (ComplexObject con matriz 75x75)\n";
    std::cout << "[demo]    - " << live.size() << " Blobs sin liberar\n";
    std::cout << "[demo] sleeping 10s — HAZ SNAPSHOT DESDE LA GUI AHORA!\n";
    std::this_thread::sleep_for(10s);

    // Limpieza final (opcional - puedes comentar esto para ver más leaks)
    delete test2;
    delete test3;
    delete complex2;

    // Cierra el cliente (los objetos que quedaron en 'live' quedan como leak a propósito)
    client.stop();
    std::cout << "[demo] done.\n";
    return 0;
}