#include "DataLoader.h"
#include <iostream>
#include <fstream>

EventType getEventType(const string& typeStr) {
    if (typeStr == "LOGIN") return LOGIN;
    if (typeStr == "LOGOUT") return LOGOUT;
    if (typeStr == "TOKEN_REFRESH") return TOKEN_REFRESH;
    if (typeStr == "ACCESS") return ACCESS;
    if (typeStr == "FAILED_LOGIN") return FAILED_LOGIN;
    if (typeStr == "OPEN_APP") return OPEN_APP;
    if (typeStr == "DOWNLOAD") return DOWNLOAD;
    if (typeStr == "ADMIN_ACTION") return ADMIN_ACTION;
    return UNKNOWN_EVENT;
}

Location getLocation(const string& locStr) {
    if (locStr == "US") return US;
    if (locStr == "VN") return VN;
    if (locStr == "JP") return JP;
    if (locStr == "KR") return KR;
    if (locStr == "SG") return SG;
    if (locStr == "CN") return CN;
    if (locStr == "DE") return DE;
    if (locStr == "FR") return FR;
    if (locStr == "UK") return UK;
    if (locStr == "AU") return AU;
    if (locStr == "CA") return CA;
    if (locStr == "IN") return IN;
    if (locStr == "BR") return BR;
    if (locStr == "RU") return RU;
    if (locStr == "TH") return TH;
    return UNKNOWN_LOC;
}

void loadCSVData(const string& filename, LogPool& pool) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return;
    }

    string line;
    getline(file, line);

    while (getline(file, line)) {
        if (line.empty()) continue; 

        LogEntry* entry = pool.allocateLog();
        
        size_t pos = 0;
        size_t next_pos = 0;
        
        next_pos = line.find(',', pos);
        entry->user_id = line.substr(pos, next_pos - pos);
        pos = next_pos + 1;

        next_pos = line.find(',', pos);
        entry->device_id = line.substr(pos, next_pos - pos);
        pos = next_pos + 1;

        next_pos = line.find(',', pos);
        entry->app_id = line.substr(pos, next_pos - pos);
        pos = next_pos + 1;

        next_pos = line.find(',', pos);
        entry->resource_id = line.substr(pos, next_pos - pos);
        pos = next_pos + 1;

        next_pos = line.find(',', pos);
        entry->event_type = getEventType(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        next_pos = line.find(',', pos);
        entry->location = getLocation(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        entry->timestamp = stoll(line.substr(pos));
    }

    file.close();
    cout << "Loading successful: " << pool.total_logs << " logs loaded into memory." << endl;
}