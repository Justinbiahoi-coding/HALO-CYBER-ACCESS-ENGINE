#include "DataLoader.h"
#include <iostream>
#include <fstream>

EventType getEventType(const string& s) {
    if (s == "LOGIN")        return LOGIN;
    if (s == "LOGOUT")       return LOGOUT;
    if (s == "TOKEN_REFRESH")return TOKEN_REFRESH;
    if (s == "ACCESS")       return ACCESS;
    if (s == "FAILED_LOGIN") return FAILED_LOGIN;
    if (s == "OPEN_APP")     return OPEN_APP;
    if (s == "DOWNLOAD")     return DOWNLOAD;
    if (s == "ADMIN_ACTION") return ADMIN_ACTION;
    return UNKNOWN_EVENT;
}

Location getLocation(const string& s) {
    if (s == "US") return US;
    if (s == "VN") return VN;
    if (s == "JP") return JP;
    if (s == "KR") return KR;
    if (s == "SG") return SG;
    if (s == "CN") return CN;
    if (s == "DE") return DE;
    if (s == "FR") return FR;
    if (s == "UK") return UK;
    if (s == "AU") return AU;
    if (s == "CA") return CA;
    if (s == "IN") return IN;
    if (s == "BR") return BR;
    if (s == "RU") return RU;
    if (s == "TH") return TH;
    return UNKNOWN_LOC;
}

//  Returns true if exactly 7 separated fields were found.
//  Writes pointers+lengths into field[0..6] to avoid allocating substrings
static const int EXPECTED_FIELDS = 7;

static bool splitLine(const string& line,
                      const char* fields[EXPECTED_FIELDS],
                      size_t      lengths[EXPECTED_FIELDS])
{
    const char* p   = line.c_str();
    const char* end = p + line.size();
    int col = 0;
    fields[0] = p;

    while (p <= end && col < EXPECTED_FIELDS) {
        if (p == end || *p == ',') {
            lengths[col] = (size_t)(p - fields[col]);
            col++;
            if (col < EXPECTED_FIELDS) fields[col] = p + 1;
        }
        p++;
    }
    return col == EXPECTED_FIELDS;
}

//  LOAD CSV
void loadCSVData(const string& filename, LogPool& pool) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[ERROR] Cannot open file: " << filename << endl;
        return;
    }

    string line;
    getline(file, line); // skip header row

    size_t skipped = 0;
    const char* fields[EXPECTED_FIELDS];
    size_t      lengths[EXPECTED_FIELDS];

    while (getline(file, line)) {
        if (line.empty()) continue;

        if (!splitLine(line, fields, lengths)) {
            skipped++;
            continue; // malformed row skip instead of crash
        }

        LogEntry* e = pool.allocateLog();

        // Build temporary strings only for ID parsing 
        string f0(fields[0], lengths[0]); e->user_id     = parseUserId    (f0);
        string f1(fields[1], lengths[1]); e->device_id   = parseDeviceId  (f1);
        string f2(fields[2], lengths[2]); e->app_id      = parseAppId     (f2);
        string f3(fields[3], lengths[3]); e->resource_id = parseResourceId(f3);
        string f4(fields[4], lengths[4]); e->event_type  = getEventType   (f4);
        string f5(fields[5], lengths[5]); e->location    = getLocation    (f5);

        // Parse timestamp no string constructor needed
        e->timestamp = (uint32_t)strtoul(fields[6], nullptr, 10);
    }

    file.close();

    if (skipped > 0)
        cerr << "[WARN] Skipped " << skipped << " malformed rows." << endl;

    cout << "[INFO] Loaded " << pool.total_logs << " logs ("
         << pool.total_logs * sizeof(LogEntry) / (1024*1024) << " MB in pool)." << endl;
}
