#ifndef HALO_ENGINE_H
#define HALO_ENGINE_H

#include <string>
#include <sstream>
#include "LogData.h"
#include "HashTable.h"
#include "DataLoader.h"

using namespace std;

struct ResourceCount {
    uint32_t resource_id;
    int      count;
    ResourceCount() : resource_id(0), count(0) {}
};

struct HaloEngine {
    LogPool    pool;
    HashTable  userTable;
    HashTable  resourceTable;
    SkipReport skipReport;      // lưu mẫu các dòng bị bỏ qua

    HaloEngine();

    void loadData(const string& filename);

    LogList* getLogsByUser    (const string& user_id);
    LogList* getLogsByResource(const string& resource_id);
    int findFirstLogAfter(LogList* list, long long start_time);
    int findLastLogBefore(LogList* list, long long end_time);

    void   printTop10ResourcesByTime   (long long start_time, long long end_time);
    string getTop10ResourcesByTimeJson (long long start_time, long long end_time);
    string getUserJourneyJson          (const string& user_id,     long long start_time, long long end_time);
    string getResourceJourneyJson      (const string& resource_id, long long start_time, long long end_time);
    string getQualityJson              ();   // trả về báo cáo data quality
};

#endif // HALO_ENGINE_H
