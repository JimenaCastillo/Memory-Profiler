#include "MemoryTracker.hpp"
#include <new> // std::nothrow (por si se usa en el futuro)
#include "ReentryGuard.hpp"  // para ScopedHookGuard

namespace mp {

// === Helpers est�ticos ===
uint64_t MemoryTracker::nowNs() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

uint32_t MemoryTracker::thisThreadId() {
    auto h = std::hash<std::thread::id>{}(std::this_thread::get_id());
    // Reducir a 32 bits de forma portable
    return static_cast<uint32_t>(h & 0xFFFFFFFFu);
}

// === Singleton ===
MemoryTracker& MemoryTracker::instance() {
    static MemoryTracker inst;
    return inst;
}

// === Registro de asignaci�n ===
void MemoryTracker::onAlloc(void* p, std::size_t sz, const char* type,
                            const char* file, int line, bool isArray) {
    if (!p || sz == 0) {
        // Si malloc devolvi� nullptr (o size==0), no registramos.
        return;
    }

    AllocationRecord rec;
    rec.ptr          = p;
    rec.size         = sz;
    rec.type_name    = type;
    rec.timestamp_ns = nowNs();
    rec.thread_id    = thisThreadId();
    rec.file         = file;
    rec.line         = line;
    rec.is_array     = isArray;

    std::lock_guard<std::mutex> lock(mu_);

    live_.emplace(p, rec);

    ++total_allocs_;
    ++active_allocs_;
    total_bytes_  += sz;
    active_bytes_ += sz;
    if (active_bytes_ > peak_bytes_) {
        peak_bytes_ = active_bytes_;
    }
}

// === Registro de liberaci�n ===
void MemoryTracker::onFree(void* p, bool /*isArray*/) noexcept {
    if (!p) return;

    std::lock_guard<std::mutex> lock(mu_);

    auto it = live_.find(p);
    if (it != live_.end()) {
        const auto sz = it->second.size;
        if (active_bytes_ >= sz) active_bytes_ -= sz; // robustez
        if (active_allocs_ > 0)  --active_allocs_;
        live_.erase(it);
    }
    // Importante: no lanzar excepciones aqu�.
}

// === Snapshot de bloques vivos ===
std::vector<AllocationRecord> MemoryTracker::snapshotLive() const {
    // Evita que las asignaciones internas del vector se auto-registren
    ScopedHookGuard guard;

    std::lock_guard<std::mutex> lock(mu_);
    std::vector<AllocationRecord> out;
    out.reserve(live_.size());
    for (const auto& kv : live_) out.push_back(kv.second);
    return out;
}

// === M�tricas ===
std::size_t MemoryTracker::activeBytes() const {
    std::lock_guard<std::mutex> lock(mu_);
    return active_bytes_;
}

std::size_t MemoryTracker::peakBytes() const {
    std::lock_guard<std::mutex> lock(mu_);
    return peak_bytes_;
}

std::size_t MemoryTracker::totalAllocs() const {
    std::lock_guard<std::mutex> lock(mu_);
    return total_allocs_;
}

std::size_t MemoryTracker::activeAllocs() const {
    std::lock_guard<std::mutex> lock(mu_);
    return active_allocs_;
}

void MemoryTracker::resetForTesting() {
    // Por ahora no-op. // TODO: permitir reset opcional bajo flag de desarrollo.
}

} // namespace mp
