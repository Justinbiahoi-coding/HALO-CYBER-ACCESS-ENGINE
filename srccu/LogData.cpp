#include "LogData.h"
#include <cstdlib>  

string zeroPad(uint32_t num, int width) {
    string s = to_string(num);
    while ((int)s.size() < width) s = "0" + s;
    return s;
}

static uint32_t parseIdNumber(const string& s, int prefixLen) {
    if ((int)s.size() <= prefixLen) return 0;
    return (uint32_t)strtoul(s.c_str() + prefixLen, nullptr, 10);
}

uint32_t parseUserId    (const string& s) { return parseIdNumber(s, 1); } // "U#####"
uint16_t parseDeviceId  (const string& s) { return (uint16_t)parseIdNumber(s, 1); } // "D#####"
uint16_t parseAppId     (const string& s) { return (uint16_t)parseIdNumber(s, 3); } // "APP####"
uint16_t parseResourceId(const string& s) { return (uint16_t)parseIdNumber(s, 1); } // "R#####"

// Reconstruct 
string userIdToStr    (uint32_t id) { return "U"   + zeroPad(id, 5); }
string deviceIdToStr  (uint16_t id) { return "D"   + zeroPad(id, 5); }
string appIdToStr     (uint16_t id) { return "APP" + zeroPad(id, 4); }
string resourceIdToStr(uint16_t id) { return "R"   + zeroPad(id, 5); }

//  MEMORY POOL
LogChunk::LogChunk() {
    entries = new LogEntry[CHUNK_SIZE];
    count   = 0;
    next    = nullptr;
}

LogChunk::~LogChunk() {
    delete[] entries;
}

LogPool::LogPool() {
    head       = new LogChunk();
    tail       = head;
    total_logs = 0;
}

LogPool::~LogPool() {
    LogChunk* current = head;
    while (current != nullptr) {
        LogChunk* nextChunk = current->next;
        delete current;
        current = nextChunk;
    }
}

LogEntry* LogPool::allocateLog() {
    if (tail->count == CHUNK_SIZE) {
        tail->next = new LogChunk();
        tail       = tail->next;
    }
    LogEntry* newLog = &tail->entries[tail->count];
    tail->count++;
    total_logs++;
    return newLog;
}
