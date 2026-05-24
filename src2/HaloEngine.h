#ifndef HALO_ENGINE_H
#define HALO_ENGINE_H

#include <string>
#include <sstream>
#include "LogData.h"
#include "HashTable.h"
#include "DataLoader.h"

using namespace std;

// struct helper to store resource access count 
struct ResourceCount {
    string resource_id;
    int count;
    
    ResourceCount() {
        resource_id = "";
        count = 0;
    }
};

struct HaloEngine {
    LogPool pool;
    HashTable userTable;
    HashTable deviceTable;
    HashTable resourceTable;

    // create HaloEngine instance
    HaloEngine();

    void loadData(const string& filename);

    LogList* getLogsByUser(const string& user_id);
    LogList* getLogsByResource(const string& resource_id);

    void printTop10ResourcesByTime(long long start_time, long long end_time);
    // api function to return JSON string 
    string getTop10ResourcesByTimeJson(long long start_time, long long end_time);
    string getUserJourneyJson(string user_id, long long start_time, long long end_time);
    string getResourceJourneyJson(string resource_id, long long start_time, long long end_time);
    int findFirstLogAfter(LogList* list, long long start_time);
    int findLastLogBefore(LogList* list, long long end_time);

};

#endif