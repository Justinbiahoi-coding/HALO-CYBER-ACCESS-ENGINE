#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <string>
#include "LogData.h"

using namespace std;

// ════════════════════════════════════════════════════════════════════════════
//  SKIP REPORT — lưu mẫu các dòng bị bỏ qua để hiển thị trên dashboard
// ════════════════════════════════════════════════════════════════════════════

struct SkipSample {
    string line;    // nội dung dòng gốc
    string reason;  // lý do bị skip
    SkipSample() {}
    SkipSample(const string& l, const string& r) : line(l), reason(r) {}
};

struct SkipSampleArray {
    SkipSample* data;
    int         count;
    int         maxKeep;    // chỉ lưu tối đa maxKeep mẫu để tránh tốn RAM

    explicit SkipSampleArray(int max = 300)
        : count(0), maxKeep(max) { data = new SkipSample[max]; }

    ~SkipSampleArray() { delete[] data; }

    void push(const string& line, const string& reason) {
        if (count < maxKeep) data[count++] = SkipSample(line, reason);
    }
};

struct SkipReport {
    SkipSampleArray malformed;   // sai số cột
    SkipSampleArray invalid;     // event/location sai, ID rỗng, timestamp = 0
    SkipSampleArray duplicate;   // trùng lặp

    size_t total_malformed;
    size_t total_invalid;
    size_t total_duplicate;

    SkipReport()
        : malformed(300), invalid(300), duplicate(300),
          total_malformed(0), total_invalid(0), total_duplicate(0) {}
};

// ════════════════════════════════════════════════════════════════════════════
EventType getEventType(const string& s);
Location  getLocation (const string& s);

// report có thể truyền nullptr nếu không cần thu thập mẫu
void loadCSVData(const string& filename, LogPool& pool, SkipReport* report = nullptr);

#endif // DATA_LOADER_H
