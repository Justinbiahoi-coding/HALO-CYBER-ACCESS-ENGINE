#include "LogData.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

// ════════════════════════════════════════════════════════════════════════════
//  GLOBAL INTERN TABLES  — khởi tạo một lần, dùng suốt vòng đời chương trình
// ════════════════════════════════════════════════════════════════════════════
StringIntern g_userIntern    (131072);   // ~18 vạn user
StringIntern g_deviceIntern  (65536);
StringIntern g_appIntern     (16384);
StringIntern g_resourceIntern(65536);

// ════════════════════════════════════════════════════════════════════════════
//  StringIntern — Implementation
// ════════════════════════════════════════════════════════════════════════════

StringIntern::StringIntern(int initialCap)
    : count(0), cap(initialCap), revCap(4096)
{
    table = new Entry[cap];     // zero-init via Entry default ctor
    rev   = new char*[revCap];
}

StringIntern::~StringIntern() {
    // Mỗi key chỉ được lưu 1 lần trong table (không nhân đôi trong rev)
    for (int i = 0; i < cap; i++) {
        if (table[i].occupied)
            delete[] table[i].key;
    }
    delete[] table;
    delete[] rev;   // con trỏ đã được giải phóng ở trên, chỉ cần xoá mảng
}

// FNV-1a hash — nhanh và ít collision
unsigned int StringIntern::hashStr(const char* s, size_t len) const {
    unsigned int h = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        h ^= (unsigned char)s[i];
        h *= 16777619u;
    }
    return h % (unsigned int)cap;
}

void StringIntern::growRev() {
    int    newCap = revCap * 2;
    char** newRev = new char*[newCap];
    for (int i = 0; i < revCap; i++) newRev[i] = rev[i];
    delete[] rev;
    rev    = newRev;
    revCap = newCap;
}

void StringIntern::rehash() {
    int    oldCap   = cap;
    Entry* oldTable = table;

    cap   = oldCap * 2;
    table = new Entry[cap];     // mặc định: isOccupied = false

    for (int i = 0; i < oldCap; i++) {
        if (!oldTable[i].occupied) continue;
        size_t       len = strlen(oldTable[i].key);
        unsigned int idx = hashStr(oldTable[i].key, len);
        // Linear probing trong bảng mới
        while (table[idx].occupied)
            idx = (idx + 1) % (unsigned int)cap;
        // Chuyển ownership con trỏ, không cấp phát lại
        table[idx].key      = oldTable[i].key;
        table[idx].index    = oldTable[i].index;
        table[idx].occupied = true;
    }

    delete[] oldTable;  // các key đã chuyển, không xoá key ở đây
}

uint32_t StringIntern::intern(const char* s, size_t len) {
    // Rehash khi load factor > 0.70
    if ((float)count / (float)cap >= 0.70f) rehash();

    unsigned int idx   = hashStr(s, len);
    unsigned int start = idx;

    // Tìm slot trống hoặc slot chứa cùng key
    while (table[idx].occupied) {
        // So sánh độ dài trước để tránh strcmp chi phí cao
        if (strlen(table[idx].key) == len &&
            memcmp(table[idx].key, s, len) == 0)
        {
            return table[idx].index;    // đã có, trả index
        }
        idx = (idx + 1) % (unsigned int)cap;
        if (idx == start) {             // bảng đầy (không nên xảy ra với rehash)
            rehash();
            idx   = hashStr(s, len);
            start = idx;
        }
    }

    // Chuỗi mới — cấp phát và lưu
    char* copy = new char[len + 1];
    memcpy(copy, s, len);
    copy[len] = '\0';

    uint32_t newIdx = count++;
    if ((int)newIdx >= revCap) growRev();

    table[idx].key      = copy;
    table[idx].index    = newIdx;
    table[idx].occupied = true;
    rev[newIdx]         = copy;         // rev trỏ vào cùng bộ nhớ với table

    return newIdx;
}

const char* StringIntern::lookup(uint32_t idx) const {
    if (idx >= count) return "";
    return rev[idx];
}

// ════════════════════════════════════════════════════════════════════════════
//  MEMORY POOL
// ════════════════════════════════════════════════════════════════════════════

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
    LogChunk* cur = head;
    while (cur) {
        LogChunk* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
}

LogEntry* LogPool::allocateLog() {
    if (tail->count == CHUNK_SIZE) {
        tail->next = new LogChunk();
        tail       = tail->next;
    }
    LogEntry* e = &tail->entries[tail->count++];
    total_logs++;
    return e;
}

void LogPool::deallocateLastLog() {
    // Huỷ log cuối khi validation thất bại — không giải phóng bộ nhớ thật,
    // chỉ lùi con trỏ (chunk là arena, không hỗ trợ free từng phần tử).
    if (total_logs == 0) return;
    if (tail->count > 0) {
        tail->count--;
        total_logs--;
    }
}
