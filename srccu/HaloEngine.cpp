#include "HaloEngine.h"
#include <iostream>

using namespace std;

//preallocate hash tables
HaloEngine::HaloEngine()
    : userTable(100000), resourceTable(100000) {}

// read CSV into pool, then build two hash-table indexes
void HaloEngine::loadData(const string& filename) {
    loadCSVData(filename, pool);

    LogChunk* chunk = pool.head;
    while (chunk != nullptr) {
        for (int i = 0; i < chunk->count; i++) {
            LogEntry* e = &chunk->entries[i];

            // Keys are reconstructed strings stored once per unique entity
            userTable    .insert(userIdToStr    (e->user_id),     e);
            resourceTable.insert(resourceIdToStr(e->resource_id), e);
        }
        chunk = chunk->next;
    }
}

//  BASIC LOOKUPS
LogList* HaloEngine::getLogsByUser(const string& user_id) {
    return userTable.search(user_id);
}

LogList* HaloEngine::getLogsByResource(const string& resource_id) {
    return resourceTable.search(resource_id);
}

//  BINARY SEARCH HELPERS  (list must be sorted before calling)
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

//  TOP 10 RESOURCES  insertion into a fixed 10-slot sorted array
static void insertTop10(ResourceCount top10[10], uint16_t res_id, int cnt) {
    // Only enter if count beats the smallest slot or ties with smaller id
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
        logs->sort(); // Fix 1: no-op on subsequent calls

        int first = findFirstLogAfter(logs, start_time);
        int last  = findLastLogBefore(logs, end_time);
        if (first == -1 || last == -1 || first > last) continue;

        int cnt    = last - first + 1;
        uint16_t r = logs->data[first]->resource_id; // all entries share the same resource_id
        insertTop10(top10, r, cnt);
    }

    cout << "\n=== TOP 10 RESOURCES [" << start_time << " – " << end_time << "] ===\n";
    bool found = false;
    for (int i = 0; i < 10; i++) {
        if (top10[i].count > 0) {
            found = true;
            cout << i+1 << ". " << resourceIdToStr(top10[i].resource_id)
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

        int cnt    = last - first + 1;
        uint16_t r = logs->data[first]->resource_id;
        insertTop10(top10, r, cnt);
    }

    string json = "[";
    bool isFirst = true;
    for (int i = 0; i < 10; i++) {
        if (top10[i].count == 0) continue;
        if (!isFirst) json += ",";
        json += "{\"resource_id\":\"" + resourceIdToStr(top10[i].resource_id) +
                "\",\"count\":"       + to_string(top10[i].count) + "}";
        isFirst = false;
    }
    json += "]";
    return json;
}

//  USER JOURNEY
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
        oss << "{\"timestamp\":"  << e->timestamp
            << ",\"device_id\":\"" << deviceIdToStr  (e->device_id)   << "\""
            << ",\"app_id\":\""    << appIdToStr     (e->app_id)      << "\""
            << ",\"resource_id\":\"" << resourceIdToStr(e->resource_id) << "\"}";
    }
    oss << "]";
    return oss.str();
}

//  RESOURCE JOURNEY
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
        oss << "{\"timestamp\":"  << e->timestamp
            << ",\"user_id\":\""   << userIdToStr    (e->user_id)     << "\""
            << ",\"device_id\":\"" << deviceIdToStr  (e->device_id)   << "\""
            << ",\"app_id\":\""    << appIdToStr     (e->app_id)      << "\"}";
    }
    oss << "]";
    return oss.str();
}
