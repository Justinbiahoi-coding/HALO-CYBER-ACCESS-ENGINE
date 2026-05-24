#ifndef LOG_DATA_H
#define LOG_DATA_H

#include <string>
#include <cstdint>
using namespace std;
enum EventType : uint8_t {
    LOGIN, LOGOUT, TOKEN_REFRESH, ACCESS,
    FAILED_LOGIN, OPEN_APP, DOWNLOAD, ADMIN_ACTION, UNKNOWN_EVENT
};

enum Location : uint8_t {
    US, VN, JP, KR, SG, CN, DE, FR, UK, AU, CA, IN, BR, RU, TH, UNKNOWN_LOC
};

struct LogEntry {
    uint32_t user_id;       
    uint16_t device_id;     
    uint16_t app_id;        
    uint16_t resource_id;   
    EventType event_type;   
    Location  location;     
    uint32_t  timestamp;    
};

string zeroPad(uint32_t num, int width);

uint32_t parseUserId    (const string& s); // "U03649"  → 3649
uint16_t parseDeviceId  (const string& s); // "D00205"  → 205
uint16_t parseAppId     (const string& s); // "APP0380" → 380
uint16_t parseResourceId(const string& s); // "R02254"  → 2254

string userIdToStr    (uint32_t id); // 3649 → "U03649"
string deviceIdToStr  (uint16_t id); // 205  → "D00205"
string appIdToStr     (uint16_t id); // 380  → "APP0380"
string resourceIdToStr(uint16_t id); // 2254 → "R02254"

//  MEMORY POOL
const int CHUNK_SIZE = 1000000; // 1 million logs per chunk

struct LogChunk {
    LogEntry* entries;
    int       count;
    LogChunk* next;

    LogChunk();
    ~LogChunk();
};

struct LogPool {
    LogChunk* head;
    LogChunk* tail;
    size_t    total_logs;

    LogPool();
    ~LogPool();
    LogEntry* allocateLog();
};

#endif
