#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include "LogData.h"

using namespace std;
struct LogList {
    LogEntry** data; // array of LogEntry pointers
    int capacity;
    int count;

    LogList(int cap = 10);
    ~LogList();
    void push_back(LogEntry* log);
    void sort(); 
private:
    void quickSort(int low, int high);
};

// node
struct HashNode {
    string key;
    LogList* logs; // logs belonging to this key
    bool isOccupied;

    HashNode();
};

//Hash Table
struct HashTable {
    HashNode* table;
    int capacity;
    int size;

    HashTable(int cap);
    ~HashTable();

    unsigned int hashFunction(const string& key);
    void insert(const string& key, LogEntry* log);
    LogList* search(const string& key);
    private:
    void rehash();
};

#endif