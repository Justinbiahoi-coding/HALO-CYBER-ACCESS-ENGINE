#include "DataLoader.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

// ════════════════════════════════════════════════════════════════════════════
//  ENUM LOOKUP  — so sánh thẳng trên buffer, không tạo std::string
// ════════════════════════════════════════════════════════════════════════════

// So sánh buffer độ dài len với chuỗi literal
static inline bool match(const char* buf, size_t len, const char* lit) {
    size_t litLen = strlen(lit);
    return len == litLen && memcmp(buf, lit, len) == 0;
}

static EventType parseEventType(const char* s, size_t len) {
    switch (len) {
        case 5:
            if (match(s, len, "LOGIN"))  return LOGIN;
            break;
        case 6:
            if (match(s, len, "LOGOUT")) return LOGOUT;
            if (match(s, len, "ACCESS")) return ACCESS;
            break;
        case 8:
            if (match(s, len, "DOWNLOAD"))   return DOWNLOAD;
            if (match(s, len, "OPEN_APP"))   return OPEN_APP;
            break;
        case 12:
            if (match(s, len, "FAILED_LOGIN")) return FAILED_LOGIN;
            if (match(s, len, "ADMIN_ACTION")) return ADMIN_ACTION;
            break;
        case 13:
            if (match(s, len, "TOKEN_REFRESH")) return TOKEN_REFRESH;
            break;
        default: break;
    }
    return UNKNOWN_EVENT;
}

static Location parseLocation(const char* s, size_t len) {
    if (len != 2) return UNKNOWN_LOC;
    // Dùng bảng tra 2 ký tự để tránh strcmp
    struct { char a, b; Location loc; } lut[] = {
        {'U','S',US}, {'V','N',VN}, {'J','P',JP}, {'K','R',KR},
        {'S','G',SG}, {'C','N',CN}, {'D','E',DE}, {'F','R',FR},
        {'U','K',UK}, {'A','U',AU}, {'C','A',CA}, {'I','N',IN},
        {'B','R',BR}, {'R','U',RU}, {'T','H',TH},
    };
    for (auto& e : lut) {
        if (s[0] == e.a && s[1] == e.b) return e.loc;
    }
    return UNKNOWN_LOC;
}

// Các overload string giữ lại để DataLoader.h compile bình thường
EventType getEventType(const string& s) { return parseEventType(s.c_str(), s.size()); }
Location  getLocation (const string& s) { return parseLocation (s.c_str(), s.size()); }

// ════════════════════════════════════════════════════════════════════════════
//  CSV SPLITTER  — tách tại chỗ, không cấp phát chuỗi con
// ════════════════════════════════════════════════════════════════════════════
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

// ════════════════════════════════════════════════════════════════════════════
//  DUPLICATE DETECTION  — hash (user_id, timestamp, event_type) → bool
//
//  Dùng bảng hash mở riêng để không tốn thêm thư viện.
//  Kích thước 4M slot → load factor ~0.4 trên 1.5M dòng.
// ════════════════════════════════════════════════════════════════════════════
struct DupeTable {
    static const int CAP = 1 << 22; // 4,194,304 slot

    struct Slot {
        uint32_t user_id;
        uint32_t timestamp;
        uint8_t  event_type;
        bool     occupied;
        Slot() : user_id(0), timestamp(0), event_type(0), occupied(false) {}
    };

    Slot* slots;
    DupeTable()  { slots = new Slot[CAP]; }
    ~DupeTable() { delete[] slots; }

    // Trả true nếu đây là dòng trùng (đã thấy rồi)
    bool isDuplicate(uint32_t uid, uint32_t ts, uint8_t ev) {
        unsigned int h = (uid * 2654435761u ^ ts * 40503u ^ ev * 6271u) & (CAP - 1);
        unsigned int start = h;
        while (slots[h].occupied) {
            if (slots[h].user_id    == uid &&
                slots[h].timestamp  == ts  &&
                slots[h].event_type == ev)
                return true;
            h = (h + 1) & (CAP - 1);
            if (h == start) return false; // bảng đầy, bỏ qua check
        }
        slots[h].user_id    = uid;
        slots[h].timestamp  = ts;
        slots[h].event_type = ev;
        slots[h].occupied   = true;
        return false;
    }
};

// ════════════════════════════════════════════════════════════════════════════
//  LOAD CSV
// ════════════════════════════════════════════════════════════════════════════
void loadCSVData(const string& filename, LogPool& pool, SkipReport* report) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "[ERROR] Cannot open file: " << filename << endl;
        return;
    }

    // Tăng buffer đọc file để giảm syscall
    const int BUF_SIZE = 1 << 20; // 1 MB
    char* buf = new char[BUF_SIZE];
    file.rdbuf()->pubsetbuf(buf, BUF_SIZE);

    string line;
    getline(file, line); // bỏ dòng header

    size_t skipped_malformed = 0;
    size_t skipped_invalid   = 0;
    size_t skipped_dupe      = 0;

    const char* fields[EXPECTED_FIELDS];
    size_t      lengths[EXPECTED_FIELDS];

    // DupeTable cấp phát trên heap (32 MB) — tự giải phóng khi ra khỏi scope
    DupeTable dupeTable;

    while (getline(file, line)) {
        // Bỏ \r nếu file CRLF (Windows)
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        // ── Kiểm tra cấu trúc ──────────────────────────────────────────────
        if (!splitLine(line, fields, lengths)) {
            skipped_malformed++;
            if (report) {
                report->total_malformed++;
                report->malformed.push(line, "Wrong column count");
            }
            continue;
        }

        // ── Kiểm tra các trường ID không được rỗng ─────────────────────────
        if (lengths[0] == 0 || lengths[1] == 0 ||
            lengths[2] == 0 || lengths[3] == 0) {
            skipped_invalid++;
            if (report) {
                report->total_invalid++;
                report->invalid.push(line, "Empty ID field");
            }
            continue;
        }

        // ── Kiểm tra event_type và location hợp lệ ─────────────────────────
        EventType evType = parseEventType(fields[4], lengths[4]);
        Location  loc    = parseLocation (fields[5], lengths[5]);
        if (evType == UNKNOWN_EVENT) {
            skipped_invalid++;
            if (report) {
                report->total_invalid++;
                string ev(fields[4], lengths[4]);
                report->invalid.push(line, "Unknown event_type: " + ev);
            }
            continue;
        }
        if (loc == UNKNOWN_LOC) {
            skipped_invalid++;
            if (report) {
                report->total_invalid++;
                string lc(fields[5], lengths[5]);
                report->invalid.push(line, "Unknown location: " + lc);
            }
            continue;
        }

        // ── Kiểm tra timestamp ─────────────────────────────────────────────
        char* tsEnd = nullptr;
        unsigned long tsRaw = strtoul(fields[6], &tsEnd, 10);
        // Chỉ reject timestamp không parse được hoặc bằng 0
        if (tsEnd == fields[6] || tsRaw == 0) {
            skipped_invalid++;
            if (report) {
                report->total_invalid++;
                string ts(fields[6], lengths[6]);
                report->invalid.push(line, "Invalid timestamp: " + ts);
            }
            continue;
        }
        uint32_t ts = (uint32_t)tsRaw;

        // ── Intern ID strings  (chấp nhận MỌI định dạng) ───────────────────
        uint32_t uid = g_userIntern    .intern(fields[0], lengths[0]);
        uint32_t did = g_deviceIntern  .intern(fields[1], lengths[1]);
        uint32_t aid = g_appIntern     .intern(fields[2], lengths[2]);
        uint32_t rid = g_resourceIntern.intern(fields[3], lengths[3]);

        // ── Phát hiện và bỏ qua dòng trùng ─────────────────────────────────
        if (dupeTable.isDuplicate(uid, ts, (uint8_t)evType)) {
            skipped_dupe++;
            if (report) {
                report->total_duplicate++;
                report->duplicate.push(line, "Duplicate (user_id + timestamp + event_type)");
            }
            continue;
        }

        // ── Ghi vào pool ───────────────────────────────────────────────────
        LogEntry* e  = pool.allocateLog();
        e->user_id   = uid;
        e->device_id = did;
        e->app_id    = aid;
        e->resource_id = rid;
        e->event_type  = evType;
        e->location    = loc;
        e->timestamp   = ts;
    }

    file.close();
    delete[] buf;

    // ── Báo cáo ────────────────────────────────────────────────────────────
    if (skipped_malformed > 0)
        cerr << "[WARN] Skipped " << skipped_malformed << " malformed rows (wrong column count).\n";
    if (skipped_invalid > 0)
        cerr << "[WARN] Skipped " << skipped_invalid   << " invalid rows (bad event/location/timestamp).\n";
    if (skipped_dupe > 0)
        cerr << "[WARN] Skipped " << skipped_dupe      << " duplicate rows.\n";

    cout << "[INFO] Loaded "     << pool.total_logs << " logs ("
         << pool.total_logs * sizeof(LogEntry) / (1024*1024) << " MB in pool).\n";
    cout << "[INFO] Unique IDs — "
         << "users: "     << g_userIntern    .count
         << " | devices: "<< g_deviceIntern  .count
         << " | apps: "   << g_appIntern     .count
         << " | resources: " << g_resourceIntern.count << "\n";
}
