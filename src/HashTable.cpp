#include "HashTable.h"

//  LOG LIST
LogList::LogList(int cap) {
    capacity = cap;
    count    = 0;
    isSorted = false;
    data     = new LogEntry*[capacity];
}

LogList::~LogList() {
    delete[] data;
}

void LogList::push_back(LogEntry* log) {
    if (count == capacity) {
        int        new_cap  = capacity * 2;
        LogEntry** new_data = new LogEntry*[new_cap];
        for (int i = 0; i < count; i++) new_data[i] = data[i];
        delete[] data;
        data     = new_data;
        capacity = new_cap;
    }
    data[count++] = log;
    isSorted = false; 
}

void LogList::sort() {
    if (isSorted || count <= 1) return;
    quickSort(0, count - 1);
    isSorted = true;
}

void LogList::quickSort(int low, int high) {
    if (low >= high) return;

    // Explicit stack stores (low, high) pairs
    int  stackCap = (high - low + 1) * 2;
    int* stack    = new int[stackCap];
    int  top      = -1;

    stack[++top] = low;
    stack[++top] = high;

    while (top >= 0) {
        int h = stack[top--];
        int l = stack[top--];

        if (l >= h) continue;

        //Median of three pivot selection 
        int m = l + (h - l) / 2;

        // Sort data[l], data[m], data[h] by timestamp (3 conditional swaps)
        if (data[l]->timestamp > data[m]->timestamp) {
            LogEntry* t = data[l]; data[l] = data[m]; data[m] = t;
        }
        if (data[l]->timestamp > data[h]->timestamp) {
            LogEntry* t = data[l]; data[l] = data[h]; data[h] = t;
        }
        if (data[m]->timestamp > data[h]->timestamp) {
            LogEntry* t = data[m]; data[m] = data[h]; data[h] = t;
        }
        // data[m] is now the median move it to data[h] to use as pivot
        { LogEntry* t = data[m]; data[m] = data[h]; data[h] = t; }

        uint32_t pivot = data[h]->timestamp;
        int i = l - 1;
        for (int j = l; j < h; j++) {
            if (data[j]->timestamp <= pivot) {
                i++;
                LogEntry* t = data[i]; data[i] = data[j]; data[j] = t;
            }
        }
        { LogEntry* t = data[i+1]; data[i+1] = data[h]; data[h] = t; }
        int pi = i + 1;

        if (pi - 1 > l) { stack[++top] = l;      stack[++top] = pi - 1; }
        if (pi + 1 < h) { stack[++top] = pi + 1; stack[++top] = h;      }
    }

    delete[] stack;
}

//  HASH NODE
HashNode::HashNode() : key(""), logs(nullptr), isOccupied(false) {}

//  HASH TABLE
HashTable::HashTable(int cap) : capacity(cap), size(0) {
    table = new HashNode[capacity];
}

HashTable::~HashTable() {
    for (int i = 0; i < capacity; i++) {
        if (table[i].isOccupied && table[i].logs != nullptr)
            delete table[i].logs;
    }
    delete[] table;
}

// FNV-1a hash
unsigned int HashTable::hashFunction(const string& key) {
    unsigned int hash = 2166136261u;
    for (unsigned char c : key) {
        hash ^= c;
        hash *= 16777619u;
    }
    return hash % (unsigned int)capacity;
}

void HashTable::insert(const string& key, LogEntry* log) {
    if ((float)size / capacity >= 0.7f) rehash();

    unsigned int index = hashFunction(key);
    unsigned int start = index;

    while (table[index].isOccupied && table[index].key != key) {
        index = (index + 1) % capacity;
        if (index == start) { rehash(); index = hashFunction(key); start = index; }
    }

    if (!table[index].isOccupied) {
        table[index].key      = key;
        table[index].logs     = new LogList();
        table[index].isOccupied = true;
        size++;
    }
    table[index].logs->push_back(log);
}

LogList* HashTable::search(const string& key) {
    unsigned int index = hashFunction(key);
    unsigned int start = index;

    while (table[index].isOccupied) {
        if (table[index].key == key) return table[index].logs;
        index = (index + 1) % capacity;
        if (index == start) break;
    }
    return nullptr;
}

void HashTable::rehash() {
    int       old_cap   = capacity;
    HashNode* old_table = table;

    capacity = old_cap * 2;
    table    = new HashNode[capacity];

    for (int i = 0; i < old_cap; i++) {
        if (!old_table[i].isOccupied) continue;

        unsigned int index = hashFunction(old_table[i].key);
        while (table[index].isOccupied)
            index = (index + 1) % capacity;

        table[index].key        = old_table[i].key;
        table[index].logs       = old_table[i].logs; 
        table[index].isOccupied = true;
    }

    delete[] old_table;
}
