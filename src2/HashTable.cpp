#include "HashTable.h"

LogList::LogList(int cap) {
    capacity = cap;
    count = 0;
    data = new LogEntry*[capacity]; // allocate array of pointers to LogEntry
}

LogList::~LogList() {
    delete[] data; 
}

void LogList::push_back(LogEntry* log) {
    if (count == capacity) {
        int new_capacity = capacity * 2;
        LogEntry** new_data = new LogEntry*[new_capacity];
        for (int i = 0; i < count; i++) {
            new_data[i] = data[i];
        }
        delete[] data; 
        data = new_data;
        capacity = new_capacity;
    }
    data[count++] = log;
}

HashNode::HashNode() {
    key = "";
    logs = nullptr;
    isOccupied = false;
}

HashTable::HashTable(int cap) {
    capacity = cap;
    size = 0;
    table = new HashNode[capacity];
}

HashTable::~HashTable() {
    for (int i = 0; i < capacity; i++) {
        if (table[i].isOccupied && table[i].logs != nullptr) {
            delete table[i].logs; 
        }
    }
    delete[] table; 
}

unsigned int HashTable::hashFunction(const string& key) {
    unsigned int hash = 2166136261u;
    for (char c : key) {
        hash ^= (unsigned char)c;
        hash *= 16777619;
    }
    return hash % capacity;
}

void HashTable::insert(const string& key, LogEntry* log) {
    //if load factor >= 0.7, rehash
    if ((float)size / capacity >= 0.7) {
        rehash();
    }

    unsigned int index = hashFunction(key);
    unsigned int startIndex = index;

    // Linear Probing
    while (table[index].isOccupied && table[index].key != key) {
        index = (index + 1) % capacity;
    }

    if (!table[index].isOccupied) {
        table[index].key = key;
        table[index].logs = new LogList();
        table[index].isOccupied = true;
        size++;
    }
    
    table[index].logs->push_back(log);
}
LogList* HashTable::search(const string& key) {
    unsigned int index = hashFunction(key);
    unsigned int startIndex = index;

    while (table[index].isOccupied) {
        if (table[index].key == key) {
            return table[index].logs; // return the list of logs for this key
        }
        index = (index + 1) % capacity;
        if (index == startIndex) break; // not found 
    }
    return nullptr;
}
void LogList::quickSort(int low, int high) {
    if (low < high) {
        // Choose timestamp of last element as pivot
        long long pivot = data[high]->timestamp;
        int i = low - 1;
        
        for (int j = low; j < high; j++) {
            // ascending order
            if (data[j]->timestamp <= pivot) {
                i++;
                // swap data[i] and data[j]
                LogEntry* temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
        // swap data[i + 1] and data[high]
        LogEntry* temp = data[i + 1];
        data[i + 1] = data[high];
        data[high] = temp;
        
        int pi = i + 1; // partitioning index
        
        quickSort(low, pi - 1);
        quickSort(pi + 1, high);
    }
}

void LogList::sort() {

    if (count > 1) {
        quickSort(0, count - 1);
    }
}
void HashTable::rehash() {
    int old_capacity = capacity;
    HashNode* old_table = table;

    // duplicate capacity and create new table
    capacity = old_capacity * 2;
    table = new HashNode[capacity];

    // transfer data from old table to new table
    for (int i = 0; i < old_capacity; i++) {
        if (old_table[i].isOccupied) {
            string key = old_table[i].key;
            LogList* existing_logs = old_table[i].logs;

            unsigned int index = hashFunction(key);
            
            // Linear Probing
            while (table[index].isOccupied) {
                index = (index + 1) % capacity;
            }

            table[index].key = key;
            table[index].logs = existing_logs;
            table[index].isOccupied = true;
        }
    }

    delete[] old_table; 
}
