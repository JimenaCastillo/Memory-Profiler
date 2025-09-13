#include "Callbacks.hpp"
#include "MemoryTracker.hpp"
#include "BlockInfo.hpp"
#include "Callsite.hpp"
#include "ReentryGuard.hpp"
#include <atomic>
#include <vector>
#include <string>

namespace mp {

static std::atomic<std::uint64_t> g_alloc_id{0};
static std::atomic<std::uint64_t> g_snapshot_id{0};

void install_callbacks_with_memorytracker() {
    mp::Callbacks cb{};

    cb.onAlloc = [](void* p, std::size_t sz, const char* /*cs*/) {
        auto cs = mp::currentCallsite();
        const bool is_array = false;
        mp::MemoryTracker::instance().onAlloc(
            p, sz, cs.type_name, cs.file, cs.line, is_array
        );
        (void)g_alloc_id.fetch_add(1, std::memory_order_relaxed);
        mp::clearCallsite();
    };

    cb.onFree = [](void* p) {
        const bool is_array = false;
        mp::MemoryTracker::instance().onFree(p, is_array);
    };

    cb.bytesInUse = [] { return mp::MemoryTracker::instance().activeBytes(); };
    cb.peakBytes  = [] { return mp::MemoryTracker::instance().peakBytes();  };
    cb.allocCount = [] { return mp::MemoryTracker::instance().totalAllocs();};

    cb.snapshot   = [] { return g_snapshot_id.fetch_add(1, std::memory_order_relaxed); };

    cb.liveBlocks = [] {
        mp::ScopedHookGuard guard;
        std::vector<BlockInfo> out;
        auto recs = mp::MemoryTracker::instance().snapshotLive();
        out.reserve(recs.size());
        for (const auto& r : recs) {
            BlockInfo b{};
            b.ptr       = r.ptr;
            b.size      = r.size;
            b.alloc_id  = g_alloc_id.fetch_add(1, std::memory_order_relaxed);
            b.thread_id = r.thread_id;
            b.t_ns      = r.timestamp_ns;
            if (r.file && *r.file) {
                b.callsite = std::string(r.file) + ":" + std::to_string(r.line);
            } else {
                b.callsite = "?:0";
            }
            out.push_back(std::move(b));
        }
        return out;
    };

    mp::register_callbacks(cb);
}

} // namespace mp
