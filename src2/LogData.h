#ifndef LOG_DATA_H
#define LOG_DATA_H

#include <string>
using namespace std;

enum EventType {
    LOGIN, LOGOUT, TOKEN_REFRESH, ACCESS,
    FAILED_LOGIN, OPEN_APP, DOWNLOAD, ADMIN_ACTION, UNKNOWN_EVENT
}; 

enum Location {
    US, VN, JP, KR, SG, CN, DE, FR, UK, AU, CA, IN, BR, RU, TH, UNKNOWN_LOC
};

struct LogEntry {
    string user_id;     
    string device_id;    
    string app_id;       
    string resource_id;  
    EventType event_type;     
    Location location;       
    long long timestamp;     
};

const int CHUNK_SIZE = 1000000; // 1 million logs per chunk
 
struct LogChunk {
    LogEntry* entries;
    int count;
    LogChunk* next;

    LogChunk(); 
    ~LogChunk(); 
};

struct LogPool {
    LogChunk* head;
    LogChunk* tail;
    size_t total_logs;

    LogPool();           
    ~LogPool();           
    LogEntry* allocateLog(); 
};

#endif