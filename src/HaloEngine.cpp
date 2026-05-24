#include "HaloEngine.h"
#include <iostream>

using namespace std;

// ── Khởi tạo hash table ──────────────────────────────────────────────────────
HaloEngine::HaloEngine()
    : userTable(100000), resourceTable(100000) {}

// ── Load data và xây index ───────────────────────────────────────────────────
void HaloEngine::loadData(const string& filename) {
    loadCSVData(filename, pool, &skipReport);

    LogChunk* chunk = pool.head;
    while (chunk != nullptr) {
        for (int i = 0; i < chunk->count; i++) {
            LogEntry* e = &chunk->entries[i];

            // Khôi phục chuỗi gốc từ intern table để dùng làm key
            // Cách này hoạt động với MỌI định dạng ID
            userTable    .insert(g_userIntern    .lookup(e->user_id),     e);
            resourceTable.insert(g_resourceIntern.lookup(e->resource_id), e);
        }
        chunk = chunk->next;
    }
}

// ── Lookup cơ bản ────────────────────────────────────────────────────────────
LogList* HaloEngine::getLogsByUser(const string& user_id) {
    return userTable.search(user_id);
}

LogList* HaloEngine::getLogsByResource(const string& resource_id) {
    return resourceTable.search(resource_id);
}

// ── Binary search helpers ────────────────────────────────────────────────────
int HaloEngine::findFirstLogAfter(LogList* list, long long start_time) {
    if (!list || list->count == 0) return -1;
    int lo = 0, hi = list->count - 1, result = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if ((long long)list->data[mid]->timestamp >= start_time) { result = mid; hi = mid - 1; }
        else lo = mid + 1;
    }
    return result;
}

int HaloEngine::findLastLogBefore(LogList* list, long long end_time) {
    if (!list || list->count == 0) return -1;
    int lo = 0, hi = list->count - 1, result = -1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if ((long long)list->data[mid]->timestamp <= end_time) { result = mid; lo = mid + 1; }
        else hi = mid - 1;
    }
    return result;
}

// ── Top 10 resources — insertion sort trên mảng 10 slot ─────────────────────
static void insertTop10(ResourceCount top10[10], uint32_t res_id, int cnt) {
    if (cnt < top10[9].count) return;
    if (cnt == top10[9].count && res_id >= top10[9].resource_id) return;

    int j = 9;
    while (j > 0 && (top10[j-1].count < cnt ||
                    (top10[j-1].count == cnt && top10[j-1].resource_id > res_id))) {
        top10[j] = top10[j-1];
        j--;
    }
    top10[j].resource_id = res_id;
    top10[j].count       = cnt;
}

void HaloEngine::printTop10ResourcesByTime(long long start_time, long long end_time) {
    ResourceCount top10[10];

    for (int i = 0; i < resourceTable.capacity; i++) {
        if (!resourceTable.table[i].isOccupied) continue;
        LogList* logs = resourceTable.table[i].logs;
        logs->sort();

        int first = findFirstLogAfter(logs, start_time);
        int last  = findLastLogBefore(logs, end_time);
        if (first == -1 || last == -1 || first > last) continue;

        int      cnt = last - first + 1;
        uint32_t r   = logs->data[first]->resource_id;
        insertTop10(top10, r, cnt);
    }

    cout << "\n=== TOP 10 RESOURCES [" << start_time << " – " << end_time << "] ===\n";
    bool found = false;
    for (int i = 0; i < 10; i++) {
        if (top10[i].count > 0) {
            found = true;
            // Khôi phục chuỗi gốc qua intern table
            cout << i+1 << ". "
                 << g_resourceIntern.lookup(top10[i].resource_id)
                 << "  (" << top10[i].count << " accesses)\n";
        }
    }
    if (!found) cout << "No resources accessed in this time range.\n";
}

string HaloEngine::getTop10ResourcesByTimeJson(long long start_time, long long end_time) {
    ResourceCount top10[10];

    for (int i = 0; i < resourceTable.capacity; i++) {
        if (!resourceTable.table[i].isOccupied) continue;
        LogList* logs = resourceTable.table[i].logs;
        logs->sort();

        int first = findFirstLogAfter(logs, start_time);
        int last  = findLastLogBefore(logs, end_time);
        if (first == -1 || last == -1 || first > last) continue;

        int      cnt = last - first + 1;
        uint32_t r   = logs->data[first]->resource_id;
        insertTop10(top10, r, cnt);
    }

    string json = "[";
    bool isFirst = true;
    for (int i = 0; i < 10; i++) {
        if (top10[i].count == 0) continue;
        if (!isFirst) json += ",";
        // g_resourceIntern.lookup trả chuỗi gốc — mọi format ID đều OK
        json += "{\"resource_id\":\"";
        json += g_resourceIntern.lookup(top10[i].resource_id);
        json += "\",\"count\":";
        json += to_string(top10[i].count);
        json += "}";
        isFirst = false;
    }
    json += "]";
    return json;
}

// ── User Journey ─────────────────────────────────────────────────────────────
string HaloEngine::getUserJourneyJson(const string& user_id,
                                      long long start_time, long long end_time) {
    LogList* logs = userTable.search(user_id);
    if (!logs) return "[]";

    logs->sort();
    int first = findFirstLogAfter(logs, start_time);
    int last  = findLastLogBefore(logs, end_time);
    if (first == -1 || last == -1 || first > last) return "[]";

    ostringstream oss;
    oss << "[";
    for (int i = first; i <= last; ++i) {
        if (i > first) oss << ",";
        const LogEntry* e = logs->data[i];
        oss << "{\"timestamp\":"     << e->timestamp
            << ",\"device_id\":\""   << g_deviceIntern  .lookup(e->device_id)   << "\""
            << ",\"app_id\":\""      << g_appIntern     .lookup(e->app_id)      << "\""
            << ",\"resource_id\":\"" << g_resourceIntern.lookup(e->resource_id) << "\""
            << ",\"event_type\":"    << (int)e->event_type
            << ",\"location\":"      << (int)e->location
            << "}";
    }
    oss << "]";
    return oss.str();
}

// ── Resource Journey ──────────────────────────────────────────────────────────
string HaloEngine::getResourceJourneyJson(const string& resource_id,
                                          long long start_time, long long end_time) {
    LogList* logs = resourceTable.search(resource_id);
    if (!logs) return "[]";

    logs->sort();
    int first = findFirstLogAfter(logs, start_time);
    int last  = findLastLogBefore(logs, end_time);
    if (first == -1 || last == -1 || first > last) return "[]";

    ostringstream oss;
    oss << "[";
    for (int i = first; i <= last; ++i) {
        if (i > first) oss << ",";
        const LogEntry* e = logs->data[i];
        oss << "{\"timestamp\":"   << e->timestamp
            << ",\"user_id\":\""   << g_userIntern  .lookup(e->user_id)   << "\""
            << ",\"device_id\":\"" << g_deviceIntern.lookup(e->device_id) << "\""
            << ",\"app_id\":\""    << g_appIntern   .lookup(e->app_id)    << "\""
            << ",\"event_type\":"  << (int)e->event_type
            << ",\"location\":"    << (int)e->location
            << "}";
    }
    oss << "]";
    return oss.str();
}

// ── Data Quality Report ───────────────────────────────────────────────────────
// Escape double quotes trong chuỗi JSON
static string jsonEscape(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else                out += c;
    }
    return out;
}

static string samplesToJson(const SkipSampleArray& arr, size_t total) {
    ostringstream oss;
    oss << "{\"total\":" << total << ",\"samples\":[";
    for (int i = 0; i < arr.count; i++) {
        if (i > 0) oss << ",";
        oss << "{\"line\":\""   << jsonEscape(arr.data[i].line)   << "\""
            << ",\"reason\":\"" << jsonEscape(arr.data[i].reason) << "\"}";
    }
    oss << "]}";
    return oss.str();
}

string HaloEngine::getQualityJson() {
    ostringstream oss;
    size_t total_skipped = skipReport.total_malformed
                         + skipReport.total_invalid
                         + skipReport.total_duplicate;

    oss << "{"
        << "\"loaded\":"         << pool.total_logs   << ","
        << "\"total_skipped\":"  << total_skipped      << ","
        << "\"malformed\":"      << samplesToJson(skipReport.malformed,  skipReport.total_malformed)  << ","
        << "\"invalid\":"        << samplesToJson(skipReport.invalid,    skipReport.total_invalid)    << ","
        << "\"duplicate\":"      << samplesToJson(skipReport.duplicate,  skipReport.total_duplicate)
        << "}";
    return oss.str();
}
