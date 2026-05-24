#include "HaloEngine.h"
#include <iostream>

using namespace std;

// allocate memory for 3 hash tables
HaloEngine::HaloEngine() 
    : userTable(100000), deviceTable(100000), resourceTable(100000) {}

void HaloEngine::loadData(const string& filename) {
    //read file and add to Memory Pool
    loadCSVData(filename, pool);

    // Build index for 3 hash tables
    LogChunk* currentChunk = pool.head;
    while (currentChunk != nullptr) {
        for (int i = 0; i < currentChunk->count; i++) {
            LogEntry* entry = &(currentChunk->entries[i]);
            
            userTable.insert(entry->user_id, entry);
            deviceTable.insert(entry->device_id, entry);
            resourceTable.insert(entry->resource_id, entry);
        }
        currentChunk = currentChunk->next;
    }
}

LogList* HaloEngine::getLogsByUser(const string& user_id) {
    return userTable.search(user_id);
}

// find first log >= start_time
int HaloEngine::findFirstLogAfter(LogList* list, long long start_time) {
    if (!list || list->count == 0) return -1;
    int left = 0, right = list->count - 1;
    int result = -1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (list->data[mid]->timestamp >= start_time) {
            result = mid;      
            right = mid - 1;  
        } else {
            left = mid + 1;
        }
    }
    return result;
}

// find the last log <= end_time
int HaloEngine::findLastLogBefore(LogList* list, long long end_time) {
    if (!list || list->count == 0) return -1;
    int left = 0, right = list->count - 1;
    int result = -1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (list->data[mid]->timestamp <= end_time) {
            result = mid;      
            left = mid + 1;  
        } else {
            right = mid - 1;
        }
    }
    return result;
}


LogList* HaloEngine::getLogsByResource(const string& resource_id) {
    return resourceTable.search(resource_id);
}
// Insertion algorithm for Top 10 resources
void HaloEngine::printTop10ResourcesByTime(long long start_time, long long end_time) {
    ResourceCount top10[10];

    // Iterate through all hash resources of the table
    for (int i = 0; i < resourceTable.capacity; i++) {
        if (resourceTable.table[i].isOccupied) {
            string current_res = resourceTable.table[i].key;
            LogList* logs = resourceTable.table[i].logs;
            
            logs->sort();
            
            int first = findFirstLogAfter(logs, start_time);
            int last = findLastLogBefore(logs, end_time);
            
            int count_in_range = 0;
            if (first != -1 && last != -1 && first <= last) {
                count_in_range = last - first + 1; 
            }

            // insert into top 10 if count_in_range is greater than the smallest count in top 10
            if (count_in_range > 0) {
                if (count_in_range > top10[9].count || 
                   (count_in_range == top10[9].count && current_res < top10[9].resource_id)) {
                    
                    int j = 9;
                    while (j > 0 && 
                          (top10[j - 1].count < count_in_range || 
                          (top10[j - 1].count == count_in_range && top10[j - 1].resource_id > current_res))) {
                        top10[j] = top10[j - 1]; 
                        j--;
                    }
                    top10[j].resource_id = current_res;
                    top10[j].count = count_in_range;
                }
            }
        }
    }

    cout << "\n=== TOP 10 RESOURCES from " << start_time << " to " << end_time << " ===" << endl;
    bool found = false;
    for (int i = 0; i < 10; i++) {
        if (top10[i].count > 0) {
            found = true;
            cout << i + 1 << ". " << top10[i].resource_id 
                 << " (" << top10[i].count << " times)" << endl;
        }
    }
    if (!found) cout << "No resources were accessed in this time range." << endl;
}
string HaloEngine::getTop10ResourcesByTimeJson(long long start_time, long long end_time) {
    ResourceCount top10[10];

    // Iterate through all hash resources of the table
    for (int i = 0; i < resourceTable.capacity; i++) {
        if (resourceTable.table[i].isOccupied) {
            string current_res = resourceTable.table[i].key;
            LogList* logs = resourceTable.table[i].logs;
            
            logs->sort(); // Lazy Sort
            int first = findFirstLogAfter(logs, start_time);
            int last = findLastLogBefore(logs, end_time);
            
            int count_in_range = 0;
            if (first != -1 && last != -1 && first <= last) {
                count_in_range = last - first + 1;
            }

            if (count_in_range > 0) {
                if (count_in_range > top10[9].count || 
                   (count_in_range == top10[9].count && current_res < top10[9].resource_id)) {
                    int j = 9;
                    while (j > 0 && 
                          (top10[j - 1].count < count_in_range || 
                          (top10[j - 1].count == count_in_range && top10[j - 1].resource_id > current_res))) {
                        top10[j] = top10[j - 1]; 
                        j--;
                    }
                    top10[j].resource_id = current_res;
                    top10[j].count = count_in_range;
                }
            }
        }
    }

    //create JSON string from top10 array
    string json = "[";
    bool isFirst = true;
    for (int i = 0; i < 10; i++) {
        if (top10[i].count > 0) {
            if (!isFirst) json += ",";
            // Ép thành định dạng: {"resource_id": "R03922", "count": 22}
            json += "{\"resource_id\":\"" + top10[i].resource_id + "\",\"count\":" + to_string(top10[i].count) + "}";
            isFirst = false;
        }
    }
    json += "]";
    return json;
}

string HaloEngine::getUserJourneyJson(string user_id, long long start_time, long long end_time) {
    int index = userTable.hashFunction(user_id);
    int originalIndex = index;

    while (userTable.table[index].isOccupied) {
        if (userTable.table[index].key == user_id) {
            LogList* logs = userTable.table[index].logs;
            logs->sort(); // make sure logs are sorted by timestamp for binary search
            int first = findFirstLogAfter(logs, start_time);
            int last = findLastLogBefore(logs, end_time);

            if (first == -1 || last == -1 || first > last) return "[]"; // No logs in this time range

            string json = "[";
            bool isFirst = true;
            for (int i = first; i <= last; ++i) {
                if (!isFirst) json += ",";
                json += "{\"timestamp\":" + to_string(logs->data[i]->timestamp) + 
                        ",\"device_id\":\"" + logs->data[i]->device_id + 
                        "\",\"app_id\":\"" + logs->data[i]->app_id + 
                        "\",\"resource_id\":\"" + logs->data[i]->resource_id + "\"}";
                isFirst = false;
            }
            json += "]";
            return json;
        }
        index = (index + 1) % userTable.capacity;
        if (index == originalIndex) break;
    }
    return "[]"; // Return empty array if User not found
}

string HaloEngine::getResourceJourneyJson(string resource_id, long long start_time, long long end_time) {
    int index = resourceTable.hashFunction(resource_id);
    int originalIndex = index;

    while (resourceTable.table[index].isOccupied) {
        if (resourceTable.table[index].key == resource_id) {
            LogList* logs = resourceTable.table[index].logs;
            logs->sort(); 
            int first = findFirstLogAfter(logs, start_time);
            int last = findLastLogBefore(logs, end_time);

            if (first == -1 || last == -1 || first > last) return "[]"; 

            ostringstream oss;
            oss << "[";
            bool isFirst = true;
            for (int i = first; i <= last; ++i) {
                if (!isFirst) oss << ",";
                oss << "{\"timestamp\":" << logs->data[i]->timestamp 
                    << ",\"user_id\":\"" << logs->data[i]->user_id 
                    << "\",\"device_id\":\"" << logs->data[i]->device_id 
                    << "\",\"app_id\":\"" << logs->data[i]->app_id << "\"}";
                isFirst = false;
            }
            oss << "]";
            return oss.str();
        }
        index = (index + 1) % resourceTable.capacity;
        if (index == originalIndex) break;
    }
    return "[]";
}
