#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include "LogData.h"

using namespace std;

//  LOG LIST
struct LogList {
    LogEntry** data;
    int        capacity;
    int        count;
    bool       isSorted; 

    LogList(int cap = 10);
    ~LogList();

    void push_back(LogEntry* log);

    void sort();

private:
    void quickSort(int low, int high);
};

//  HASH NODE
struct HashNode {
    string   key;
    LogList* logs;
    bool     isOccupied;

    HashNode();
};

//  HASH TABLE open addressing with linear probing,auto-rehash at 0.7
struct HashTable {
    HashNode*    table;
    int          capacity;
    int          size;

    HashTable(int cap);
    ~HashTable();

    unsigned int hashFunction(const string& key);
    void         insert(const string& key, LogEntry* log);
    LogList*     search(const string& key);

private:
    void rehash();
};

#endif
