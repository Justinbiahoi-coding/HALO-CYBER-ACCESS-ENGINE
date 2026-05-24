#include "LogData.h"

LogChunk::LogChunk() {
    entries = new LogEntry[CHUNK_SIZE];
    count = 0;
    next = nullptr;
}

LogChunk::~LogChunk() {
    delete[] entries;
}

LogPool::LogPool() {
    head = new LogChunk();
    tail = head;
    total_logs = 0;
}

LogPool::~LogPool() {
    LogChunk* current = head;
    while (current != nullptr) {
        LogChunk* nextChunk = current->next;
        delete current;
        current = nextChunk;
    }
}

LogEntry* LogPool::allocateLog() {
    if (tail->count == CHUNK_SIZE) {
        tail->next = new LogChunk();
        tail = tail->next;
    }
    LogEntry* newLog = &tail->entries[tail->count];
    tail->count++;
    total_logs++;
    return newLog;
}