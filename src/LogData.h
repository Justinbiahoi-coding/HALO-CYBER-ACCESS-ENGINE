#ifndef LOG_DATA_H
#define LOG_DATA_H

#include <string>
#include <cstdint>
#include <cstring>
using namespace std;

// ════════════════════════════════════════════════════════════════════════════
//  STRING INTERN TABLE
//
//  Ánh xạ chuỗi bất kỳ ↔ chỉ số uint32_t compact.
//  Chấp nhận mọi định dạng ID: "U03649", "ahkbfjahf", "user-99", v.v.
//
//  - intern(s)  : trả về index (tạo mới nếu chưa có), O(1) amortized
//  - lookup(idx): trả về con trỏ chuỗi gốc,            O(1)
//  - Bộ nhớ     : mỗi chuỗi unique chỉ lưu 1 lần
// ════════════════════════════════════════════════════════════════════════════
struct StringIntern {

    // ── Internal hash node ────────────────────────────────────────────────
    struct Entry {
        char*    key;       // heap-allocated, owned by this table
        uint32_t index;
        bool     occupied;
        Entry() : key(nullptr), index(0), occupied(false) {}
    };

    Entry*   table;     // open-addressing hash table (string → index)
    char**   rev;       // index → original string pointer (reverse lookup)
    uint32_t count;     // số chuỗi unique đã intern
    int      cap;       // sức chứa của hash table
    int      revCap;    // sức chứa của mảng rev

    explicit StringIntern(int initialCap = 65536);
    ~StringIntern();

    // Nạp chuỗi vào bảng, trả về index. Thread-unsafe (single-threaded OK).
    uint32_t intern(const char* s, size_t len);
    uint32_t intern(const string& s) { return intern(s.c_str(), s.size()); }

    // Tra chuỗi gốc từ index. Trả "" nếu index không hợp lệ.
    const char* lookup(uint32_t idx) const;

private:
    unsigned int hashStr(const char* s, size_t len) const;
    void rehash();
    void growRev();
};

// ── Global intern tables (một bảng cho mỗi cột ID) ──────────────────────────
extern StringIntern g_userIntern;
extern StringIntern g_deviceIntern;
extern StringIntern g_appIntern;
extern StringIntern g_resourceIntern;

// ════════════════════════════════════════════════════════════════════════════
//  ENUMS
// ════════════════════════════════════════════════════════════════════════════
enum EventType : uint8_t {
    LOGIN, LOGOUT, TOKEN_REFRESH, ACCESS,
    FAILED_LOGIN, OPEN_APP, DOWNLOAD, ADMIN_ACTION, UNKNOWN_EVENT
};

enum Location : uint8_t {
    US, VN, JP, KR, SG, CN, DE, FR, UK, AU, CA, IN, BR, RU, TH, UNKNOWN_LOC
};

// ════════════════════════════════════════════════════════════════════════════
//  LOG ENTRY  — 24 bytes mỗi dòng
//
//  Các trường *_id không còn là số parse từ prefix.
//  Chúng là INDICES vào bảng StringIntern tương ứng.
//  Chuỗi gốc luôn có thể khôi phục qua g_xxxIntern.lookup(id).
// ════════════════════════════════════════════════════════════════════════════
struct LogEntry {
    uint32_t  user_id;       // index vào g_userIntern
    uint32_t  device_id;     // index vào g_deviceIntern
    uint32_t  app_id;        // index vào g_appIntern
    uint32_t  resource_id;   // index vào g_resourceIntern
    EventType event_type;
    Location  location;
    // 2 bytes padding ở đây (compiler tự thêm)
    uint32_t  timestamp;
};

// ════════════════════════════════════════════════════════════════════════════
//  MEMORY POOL
// ════════════════════════════════════════════════════════════════════════════
const int CHUNK_SIZE = 1000000;

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

    // Huỷ log cuối cùng đã cấp phát (dùng khi validation thất bại)
    void deallocateLastLog();
};

#endif // LOG_DATA_H
