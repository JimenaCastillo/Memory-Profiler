#include "Serializer.hpp"
#include <string>
#include <sstream>

namespace mp::serialize {

static void appendEscaped(std::string& s, const char* cstr) {
    if (!cstr) { s += "null"; return; }
    s += '"';
    for (const char* p = cstr; *p; ++p) {
        char c = *p;
        switch (c) {
            case '\\': s += "\\\\"; break;
            case '\"': s += "\\\""; break;
            case '\n': s += "\\n";  break;
            case '\r': s += "\\r";  break;
            case '\t': s += "\\t";  break;
            default:   s += c;      break;
        }
    }
    s += '"';
}

std::string metricsJson(const MemoryTracker& tracker) {
    // Desactiva el hook mientras armamos strings (evita auto-registro)
    ScopedHookGuard guard;

    std::string out;
    out.reserve(256);
    out += "{";
    out += "\"active_bytes\":";   out += std::to_string(tracker.activeBytes()); out += ",";
    out += "\"peak_bytes\":";     out += std::to_string(tracker.peakBytes());   out += ",";
    out += "\"total_allocs\":";   out += std::to_string(tracker.totalAllocs()); out += ",";
    out += "\"active_allocs\":";  out += std::to_string(tracker.activeAllocs());
    out += "}";
    return out;
}

std::string snapshotJson(const std::vector<AllocationRecord>& records) {
    ScopedHookGuard guard;

    std::string out;
    out.reserve(records.size() * 64 + 32);
    out += "[";
    bool first = true;
    for (const auto& r : records) {
        if (!first) out += ",";
        first = false;
        out += "{";
        out += "\"ptr\":";          out += std::to_string(reinterpret_cast<uintptr_t>(r.ptr)); out += ",";
        out += "\"size\":";         out += std::to_string(r.size); out += ",";
        out += "\"type_name\":";    appendEscaped(out, r.type_name); out += ",";
        out += "\"timestamp_ns\":"; out += std::to_string(r.timestamp_ns); out += ",";
        out += "\"thread_id\":";    out += std::to_string(r.thread_id); out += ",";
        out += "\"file\":";         appendEscaped(out, r.file); out += ",";
        out += "\"line\":";         out += std::to_string(r.line); out += ",";
        out += "\"is_array\":";     out += (r.is_array ? "true" : "false");
        out += "}";
    }
    out += "]";
    return out;
}

} // namespace mp::serialize
